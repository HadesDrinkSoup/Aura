// Study Game Ability System _Hades


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"	// ServerSetReplicatedTargetData / 复制数据缓存 / 目标数据委托都在这里
// 备注：FGameplayAbilityTargetData_SingleTargetHit、FGameplayAbilityTargetDataHandle 等类型来自
//       Abilities/GameplayAbilityTargetTypes.h，由 AbilityTask.h 间接包含，无需再写 include。

// ═════════════════════════════════════════════════════════════════════════════
// 第 0 步：工厂函数 —— 只做"创建对象 + 注册进技能"，不做任何业务逻辑
// 触发时机：GA_FireBall 蓝图里 TargetDataUnderMouse 节点执行时，
//           引擎的 Latent 节点（K2Node_LatentAbilityCall）调用这个 static 函数。
// ═════════════════════════════════════════════════════════════════════════════
UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	// NewAbilityTask<T>(ThisAbility) 内部（AbilityTask.h:136-147）做了三件事：
	//   1) NewObject<T>() 创建 Task 实例（UObject，生命周期受 GC/技能管理）；
	//   2) MyObj->InitTask(*ThisAbility, ThisAbility->GetGameplayTaskDefaultPriority())
	//      把 Task 挂到技能的 Task 列表上，并填充基类的 Ability / AbilitySystemComponent /
	//      TasksComponent 指针 —— 所以后面 Activate() 里能直接用这些成员；
	//   3) DebugRecordAbilityTaskCreatedByAbility() 做调试记录（统计/调试用）。
	//
	// 注意：工厂函数里不要写逻辑、不要广播委托（AbilityTask.h:31-33 的官方约定）。
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

// ═════════════════════════════════════════════════════════════════════════════
// 第 1 步：Activate() —— 任务真正开始执行的地方
// 蓝图里工厂节点执行后会自动调用 ReadyForActivation() → 内部调用本函数；
// 如果是在纯 C++ 里手工创建 Task，必须自己显式调用 ReadyForActivation()。
// ═════════════════════════════════════════════════════════════════════════════
void UTargetDataUnderMouse::Activate()
{
	// 1.1 必须调用父类：UAbilityTask::Activate() 会设置 Task 的运行/激活状态等，
	//     后面的 ShouldBroadcastAbilityTaskDelegates() 依赖这套状态。
	Super::Activate();

	// 1.2 用"是不是本地控制"把逻辑分成两条完全不同的路径：
	//     - 拥有者客户端（以及单机/ListenServer 的主机）：鼠标在本机，自己采集数据；
	//     - 服务器上为非本地控制的角色、或别的代理：没有鼠标光标，只能等客户端同步数据。
	if (Ability->GetCurrentActorInfo()->IsLocallyControlled())
	{
		// 1.3 本地控制路径：采集鼠标下的目标 → 广播 → 同步给服务器
		SendMouseCursorData();
	}
	else
	{
		// 1.4 远程路径：向 ASC 注册"目标数据已设置"的回调
		//     key = (技能实例句柄 SpecHandle, 本次激活的预测键 PredictionKey)
		//     用这个组合做 key，保证只有"本次激活、本技能"的数据才会进这个回调，不会串台。
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		// AbilityTargetDataSetDelegate(...) 语义是 FindOrAdd：返回 ASC 内部
		// AbilityTargetDataMap[SpecHandle+PredictionKey].TargetSetDelegate，并在这里挂上我们的回调。
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);

		// 1.5 处理竞态（这是最容易看漏的一步）：
		//     客户端的 ServerSetReplicatedTargetData RPC 有可能"先于"本 Task 激活而到达服务器，
		//     此时数据已经躺在 ASC 的缓存里、委托也已经广播过一次（没人听）。
		//     CallReplicatedTargetDataDelegatesIfSet() 会去查缓存（AbilitySystemComponent_Abilities.cpp:4090）：
		//        - 若数据已到：用缓存里的预测键开一个 FScopedPredictionWindow，立刻广播委托，
		//          触发我们刚注册的 OnTargetDataReplicatedCallback，返回 true；
		//        - 若数据没到：返回 false（这次的 delegate 只是被 Add，等 RPC 到了再广播）。
		const bool  bCallDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if (!bCallDelegate)
		{
			// 1.6 数据还没来：把 Task 标记为"等待远程玩家数据"
			//     内部：WaitStateBitMask |= WaitingOnUser，并 Ability->NotifyAbilityTaskWaitingOnPlayerData(this)
			//     （AbilityTask.cpp:234-241）。作用是：
			//       - 让技能知道还有一个 Task 在等远程数据，不能当作"任务已全部结束"；
			//       - 如果远端（客户端）已经把技能结束了，服务器这边会 ForceCancelAbilityDueToReplication
			//         直接把技能强制取消（GameplayAbility.cpp:2298-2308），避免技能悬挂。
			SetWaitingOnRemotePlayerData();
		}
	}
}

// ═════════════════════════════════════════════════════════════════════════════
// 第 2 步：SendMouseCursorData() —— 只在本地控制端执行：采集鼠标目标 → 同步服务器 → 本地广播
// ═════════════════════════════════════════════════════════════════════════════
void UTargetDataUnderMouse::SendMouseCursorData() const
{
	// 2.1 开一个"预测窗口"（RAII，函数结束自动关闭）。
	//     作用：让本次预测拥有有效的 ScopedPredictionKey，从而
	//       - 后续 ServerSetReplicatedTargetData 能带上正确的 CurrentPredictionKey；
	//       - 期间触发的 GameplayEffect 等也能挂到同一个预测键上做客户端预测/回滚。
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	// 2.2 取当前角色信息里的玩家控制器；再用它把"鼠标屏幕坐标"反投影到世界里做射线检测。
	//     ECC_Visibility = 只检测对 Visibility 通道有响应的物体（默认的地面/角色都会挡光）；
	//     bTraceComplex=false 用简单碰撞，性能更好；
	//     命中结果全部写进 CursorHit（HitActor、ImpactPoint、Trace Start/End、骨骼名等）。
	const APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	
	// 2.3 把命中结果包成 GAS 的目标数据结构：
	//     先用 new 造一个 FGameplayAbilityTargetData_SingleTargetHit（它带虚函数、要被 TSharedPtr 托管），
	//     填好 HitResult 后交给 DataHandle::Add()，所有权随即转交给句柄，不需要我们 delete。
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);
	
	// 2.4 把目标数据同步给服务器。
	//     ServerSetReplicatedTargetData 是一个 UFUNCTION(Server, reliable, WithValidation) 的 RPC
	//     （AbilitySystemComponent.h:1572），只在客户端拥有该 ASC 时才会真正发网络包；
	//     参数依次是：
	//       - 本技能的 SpecHandle：告诉服务器"这是哪个技能实例要的数据"；
	//       - 本次激活的预测键：服务器用它把数据配对回对应的技能激活；
	//       - DataHandle：目标数据本体；
	//       - FGameplayTag()：ApplicationTag，本类不用，留空；
	//       - ScopedPredictionKey：上面预测窗口当前生效的预测键（本次预测的"当前键"）。
	//     服务器收到后执行 ServerSetReplicatedTargetData_Implementation
	//     （AbilitySystemComponent_Abilities.cpp:4007）：写入 AbilityTargetDataMap[SpecHandle+PredictionKey]，
	//     置 bTargetConfirmed=true，并广播 TargetSetDelegate → 触发第 3 步的回调。
	//     单机/ListenServer 主机自己就是权威，这个 Server RPC 会直接本地执行，不会真的发包。
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);
	
	// 2.5 本地立刻广播 ValidData：这样技能在客户端"当场"就能拿到目标数据，
	//     不必等服务器 RPC 往返，这就是 GAS 的预测手感。
	//     ShouldBroadcastAbilityTaskDelegates() 会检查 Ability && Ability->IsActive()
	//     （AbilityTask.cpp:197-207）：技能已经结束就不再广播，避免往已销毁的蓝图图里回调。
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

// ═════════════════════════════════════════════════════════════════════════════
// 第 3 步：OnTargetDataReplicatedCallback() —— 服务器/代理端：客户端数据到达后的收尾
// 触发来源有两个：
//   a) 正常时序：客户端 RPC 到达 → ServerSetReplicatedTargetData_Implementation 广播 TargetSetDelegate
//      （AbilitySystemComponent_Abilities.cpp:4030）；
//   b) 竞态时序：数据先到，Activate() 里的 CallReplicatedTargetDataDelegatesIfSet 补广播。
// ═════════════════════════════════════════════════════════════════════════════
void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag) const
{
	// 3.1 消费（清空）ASC 里缓存的目标数据与确认标记（AbilitySystemComponent_Abilities.cpp:3900）。
	//     数据只应该被这次技能激活使用一次，不清掉的话会被下一次激活/下一次查询重复拿到。
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	// 3.2 把数据往上交给技能蓝图（与本地路径同一个输出口 ValidData）。
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
