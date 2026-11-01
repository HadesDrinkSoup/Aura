// Study Game Ability System _Hades

/**
 * @file AuraEffectActor.cpp
 * @brief 效果 Actor 实现：探测目标 ASC、按配置施加 Instant/Duration/Infinite 三类 GE、追踪并移除 Infinite 句柄。
 *
 * 要点：
 *  - 全部逻辑都建立在"调用方在服务器执行"的前提上：ApplyGameplayEffectSpecToSelf /
 *    RemoveActiveGameplayEffect 都要求 ASC 拥有权威，客户端调用只会拿到无效句柄并打引擎警告。
 *    因此 Overlap 回调的端过滤（是否只在服务器产生重叠）由蓝图侧负责，本文件不做 HasAuthority 判断。
 *  - 目标不按类型区分，统一用 IAbilitySystemInterface 探测 ASC：玩家角色、敌人、召唤物都走同一条路径，
 *    没有 ASC 的 Actor（场景装饰、投射物本身）会被静默跳过。
 *  - Instant / Duration / Infinite 的差别只有一处：是否需要追踪句柄以便将来移除。
 *    判定依据是 GE 资源的 DurationPolicy（CDO 上的静态配置），而不是它被填在哪个数组里。
 *  - 追踪表（ActiveEffectsMap）以目标 ASC 为键，只在施加效果的端有效；本类没有重写 EndPlay/Destroyed，
 *    所以本 Actor 被外部销毁时，目标身上残留的 Infinite 效果不会被自动移除。
 */

#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

// ----------------------------------------------------------------------------
// Construction
// ----------------------------------------------------------------------------

/**
 * 构造：只关闭 Tick。
 * 本类刻意不创建碰撞组件、不在构造期绑定任何委托——"生效范围"由蓝图子类里的
 * Sphere/Box 组件决定，蓝图的 OnComponentBeginOverlap / OnComponentEndOverlap
 * 再去调用 OnBeginOverlap / OnEndOverlap（二者是 BlueprintCallable）。
 * 好处是同一个 C++ 类可以同时做拾取物（小球）和伤害区域（大盒子），
 * 代价是"谁在什么端触发"完全取决于蓝图配置，C++ 侧无法兜底。
 */
AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

// ----------------------------------------------------------------------------
// ApplyEffectToTarget
// ----------------------------------------------------------------------------

/**
 * 向目标施加单个 GE，并在其为 Infinite 时登记句柄。
 *
 * 调用链：蓝图 Overlap 回调 → OnBeginOverlap/OnEndOverlap → ApplyEffectArrayToTarget → 本函数。
 * 参数语义：TargetActor 决定效果落在谁的 ASC 上；InstigatorActor 只写入 EffectContext
 * （击杀归属、GE 的 Instigator 标签判定），为空时退化为本 Actor，避免上下文缺失。
 *
 * 失败分支全部静默返回：没有 ASC（不是角色/没有 GAS）、GE 类为空、Spec 无效。
 * 这样即使 BP 里漏配一项，也不会打断同一数组里后续效果的施加。
 */
void AAuraEffectActor::ApplyEffectToTarget(AActor* InstigatorActor, AActor* TargetActor, const TSubclassOf<UGameplayEffect>& GameplayEffectClass)
{
	// 用接口查询而不是 Cast<ACharacter>：凡实现了 IAbilitySystemInterface 的 Actor 都算合法目标；
	// 玩家侧的 ASC 实际挂在 PlayerState 上，但该函数会沿接口正确取到，不需要特殊处理。
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	if (!GameplayEffectClass) return;

	// 上下文建在"目标的 ASC"上：InstigatorActor 可能为空或与目标分离（例如地面陷阱），
	// 用目标的 ASC 才能保证 SourceObject/Instigator 一定写进同一份有效上下文。
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	// SourceObject = 本 Actor：GE 的 GameplayTag 判定、部分 MMC 会读它来区分"哪瓶药/哪个区域"。
	EffectContextHandle.AddSourceObject(this);
	if (!InstigatorActor)
	{
		InstigatorActor = this;
	}
	// 两个实参分别是 Instigator（谁触发的）与 EffectCauser（由什么物体造成的），
	// 这里把 EffectCauser 记为本 Actor，便于死亡日志/伤害归因区分"谁碰的"和"碰了什么"。
	EffectContextHandle.AddInstigator(InstigatorActor, this);

	// GE 等级取 ActorLevel：只有用到按等级取值的曲线/MMC 的 GE 才会体现出差别
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	// 双重校验：Data 是共享的 Spec 对象，Def 是其中的 GE CDO；缺一不可，直接解引用会崩
	if (!EffectSpecHandle.Data.Get() || !EffectSpecHandle.Data->Def.Get()) return;

	// ApplyGameplayEffectSpecToSelf 要求本端对该 ASC 有权威：客户端调用会返回无效句柄（并在日志里告警）。
	// 服务器施加后，Instant 立即结算，Duration/Infinite 生成 ActiveGameplayEffect 并复制给客户端。
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	// Infinite 效果自动追踪句柄，供 bRemoveOnExit 在离开重叠时移除
	// （本类没有 bRemoveOnEnter：进入时只登记不移除，所以同一目标反复进入会累积句柄）
	// 判定看 GE 自身的 DurationPolicy，而不是它来自哪个数组：
	// Instant 不产生 ActiveEffect、Duration 会自行到期，二者都不需要（也无法）由本 Actor 移除。
	const bool bIsInfinite = EffectSpecHandle.Data->Def->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite)
	{
		// FindOrAdd：首次对该目标施加时自动建表项。句柄即使无效（被免疫/立即取消）也会登记，
		// 后续移除时是一次无害的空操作，换来的是"登记逻辑没有分支"。
		ActiveEffectsMap.FindOrAdd(TargetASC).Handles.Add(ActiveEffectHandle);
	}
}

// ----------------------------------------------------------------------------
// ApplyEffectArrayToTarget (helper)
// ----------------------------------------------------------------------------

/**
 * 批量施加一个数组里的所有 GE。
 * 顺序严格按数组顺序；单项失败（空类/目标无 ASC）只跳过该项，不中断后续项。
 * 因此若 GE 之间存在属性依赖（例如 Infinite 光环的 OnApplied 要读 Instant 写入的属性），
 * 必须在蓝图里自行排好数组顺序——这里不做任何排序或依赖解析。
 */
void AAuraEffectActor::ApplyEffectArrayToTarget(AActor* InstigatorActor, AActor* TargetActor, const TArray<TSubclassOf<UGameplayEffect>>& EffectClasses)
{
	for (const TSubclassOf<UGameplayEffect>& EffectClass : EffectClasses)
	{
		ApplyEffectToTarget(InstigatorActor, TargetActor, EffectClass);
	}
}

// ----------------------------------------------------------------------------
// RemoveStoredEffects (helper)
// ----------------------------------------------------------------------------

/**
 * 移除该目标 ASC 上由本 Actor 追踪的全部效果句柄。
 *
 * 只在 OnEndOverlap 且 bRemoveOnExit 为真时被调用（这是本类唯一的移除入口）。
 * 层数语义：RemoveActiveGameplayEffect 的第二个参数为 -1 表示"移除全部层数"，
 * 因此 bRemovalAllStacks 为真时传 -1，否则传 RemovalStackNumber（>=1）。
 * 对没有 Stacking 配置的 GE，剥 1 层就等于整条移除。
 *
 * 已知局限（读代码时的坑）：
 *  - 只要调用到这里，无论是否只剥了一层，表项都会被整体 Remove(TargetASC) 删除；
 *    若效果还剩层数，剩余层数就失去句柄，本 Actor 再也无法移除它们。
 *  - 同一目标反复进入重叠区域会追加多个句柄（可能指向同一个活跃效果实例），
 *    这里逐个移除，句柄失效时调用为空操作，不会崩。
 */
void AAuraEffectActor::RemoveStoredEffects(UAbilitySystemComponent* TargetASC)
{
	// 用 Find 而不是 FindOrAdd：没有登记过的目标不应该在移除时留下空表项
	if (FActiveGameplayEffectHandleArray* ActiveEffectHandles = ActiveEffectsMap.Find(TargetASC))
	{
		const int32 StacksToRemove = bRemovalAllStacks ? -1 : RemovalStackNumber;
		for (const FActiveGameplayEffectHandle& Handle : ActiveEffectHandles->Handles)
		{
			TargetASC->RemoveActiveGameplayEffect(Handle, StacksToRemove);
		}
		ActiveEffectsMap.Remove(TargetASC);
	}
}

// ----------------------------------------------------------------------------
// OnBeginOverlap
// ----------------------------------------------------------------------------

/**
 * 进入区域的回调（由蓝图在组件的 OnComponentBeginOverlap 中调用，约定只在服务器调用）。
 *
 * 施加顺序固定为 Instant → Duration → Infinite，理由：
 *  1) Instant 先结算，循环里后续 GE 的 OnApplied 若读结果属性（如"先回血再挂回复光环"）能读到最新值；
 *  2) Infinite 最后挂上，尽量缩短"永久效果已存在但前置属性还没写"的窗口；
 *  3) bDestroy 放在最后，保证一次性消耗品是"先给效果、再销毁自己"，顺序颠倒会导致什么都没给上。
 * 若 GE 之间有更强的依赖，只能靠蓝图里的数组顺序保证——本函数不做依赖解析。
 */
void AAuraEffectActor::OnBeginOverlap(AActor* InstigatorActor, AActor* TargetActor)
{
	// 1. 再施加进入效果
	if (!OnEnterInstantEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnEnterInstantEffects);
	}
	if (!OnEnterDurationEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnEnterDurationEffects);
	}
	if (!OnEnterInfiniteEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnEnterInfiniteEffects);
	}

	// 2. 销毁自身（一次性消耗品模式）
	// 拾取物常用：Destroy 是延迟到帧末处理，本帧后续代码仍可安全执行。
	// 注意一旦销毁，本次重叠就不会再有 OnEndOverlap，bRemoveOnExit 也就永远不会生效。
	if (bDestroy)
	{
		Destroy();
	}
}

// ----------------------------------------------------------------------------
// OnEndOverlap
// ----------------------------------------------------------------------------

/**
 * 离开区域的回调（由蓝图在组件的 OnComponentEndOverlap 中调用，约定只在服务器调用）。
 *
 * 执行顺序：移除已追踪效果 → OnExit 三组效果 → 可能的自我销毁。
 * 之所以先移除再施加：若顺序反过来，本次 OnExitInfiniteEffects 刚挂上的新效果
 * 会被紧随其后的 RemoveStoredEffects 当成"旧效果"一起删掉（它也会登记进同一张表）。
 *
 * 未做端过滤的原因与 OnBeginOverlap 相同：只有服务器收到重叠事件时才应走到这里；
 * 客户端即使因本地移动产生重叠，也只会因为没有权威而静默失败（外加引擎告警）。
 */
void AAuraEffectActor::OnEndOverlap(AActor* InstigatorActor, AActor* TargetActor)
{
	// 1. 先移除已追踪的旧效果（如进入时施加的毒气 DoT），再施加新效果（如永久 MaxHP-10），
	//    避免新 Infinite 效果被自己的 bRemoveOnExit 误删
	if (bRemoveOnExit)
	{
		// 这里重新探一次 ASC（而不是复用 ApplyEffectToTarget 里的那次查询）：
		// 目标可能在重叠期间被销毁（角色死亡），此时 ASC 为 null，移除逻辑应整体跳过而不是崩。
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
		{
			RemoveStoredEffects(TargetASC);
		}
	}

	// 2. 再施加离开效果（顺序同进入：Instant → Duration → Infinite）
	if (!OnExitInstantEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnExitInstantEffects);
	}
	if (!OnExitDurationEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnExitDurationEffects);
	}
	if (!OnExitInfiniteEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnExitInfiniteEffects);
	}

	// 3. 销毁自身（一次性消耗品模式）
	// 放在移除/施加之后：销毁后 ActiveEffectsMap 随 Actor 一起消失，
	// 因此"离开时销毁"的配置不会留下可回收的句柄。
	if (bDestroy)
	{
		Destroy();
	}
}
