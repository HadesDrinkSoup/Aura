// Study Game Ability System _Hades

/**
 * @file AuraAbilitySystemLibrary.cpp
 * @brief 全局查询库实现：职业配置表的取用，以及本地玩家的 UI WidgetController 获取。
 *
 * 要点：
 *  - 这些函数都是「查一下、返回指针」，不改任何状态，因此不需要复制、不需要广播，也没有服务器限制；
 *  - 配置类函数的数据源是 AAuraGameModeBase：职业表挂在 GameMode 的 BP 默认值上。
 *    因此**只在服务器有效**——`UGameplayStatics::GetGameMode` 在客户端返回 nullptr（见各函数注释）；
 *  - UI 类函数的链路是「0 号 PC → HUD → 组装 FWidgetControllerParams → HUD 的懒加载单例」，
 *    只在有本地玩家的端有效，专用服务器上返回 nullptr；
 *    例外是 GetElementHealthBarController：敌人血条 Controller 不挂在 HUD 上，
 *    而是每个敌人自己持有，所以它走「血条 Widget 自己 → 它的 WidgetController」；
 *    只有在别的对象里调用时才退回「0 号 PC → 光标目标敌人 → 敌人身上的实例」；
 *  - 战斗查询类（GetLiveCharactersWithinRadius）是唯一的非 BlueprintPure 函数：
 *    它做一次球形重叠查询并把结果写进输出数组，不改游戏状态，同样没有服务器限制。
 */

#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AuraGameplayEffectContext.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Game/AuraGameModeBase.h"
#include "Interface/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/EnemyHealthBarWidgetController.h"
#include "UI/WidgetController/AuraWidgetController.h"

/**
 * 取本地 0 号玩家主 HUD 的 WidgetController（血球/蓝球/技能图标的数据源）。
 *
 * 逐层"能取到就继续、取不到就 nullptr"：
 *   1. 0 号 PlayerController 且必须是 AAuraPlayerController（HUD 类挂在 PC 上，别的 PC 子类没有 AAuraHUD）；
 *   2. PC->GetHUD() 必须是 AAuraHUD（BP_AuraHUD 派生，持有 Controller 缓存与各 Widget 类）；
 *   3. 用 PlayerState 的 ASC/AS 组装 FWidgetControllerParams，交给 HUD 的懒加载取 Controller。
 *
 * 只在有本地玩家的端有效：专用服务器没有 PlayerController/HUD，必然返回 nullptr；
 * 客户端每个本地玩家各自持有自己的 HUD 与 Controller（由 HUD 缓存），所以这里固定取索引 0。
 * 注意 PS 没有判空：PC 存在但 PlayerState 尚未复制下来（刚连接、关卡切换中）时会解引用空指针，
 * 调用方应确保在 HUD 完成 InitOverlay 之后再调用。
 */
UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	// 取本地 0 号 PC 并校验类型；不是 Aura 的 PC 说明 HUD 也不是 AuraHUD，直接走 nullptr
	if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
	{
		// HUD 由 PC 创建并持有，Controller 缓存也在 HUD 上，所以必须经过 HUD 这一层
		if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			// 玩家角色的 ASC/AS 挂在 PlayerState 上（跨死亡保留），Widget 要监听的就是这份数据
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAuraAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAuraAttributeSet* AS = PS->GetAttributeSet();
			// Controller 需要 PC+PS 订阅 UI 事件、ASC+AS 绑定属性与 GE 回调，四者缺一不可
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return HUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	// 取不到 PC/HUD：非本地玩家端、或还未完成 HUD 初始化
	return nullptr;
}

/**
 * 取本地 0 号玩家的属性菜单 WidgetController。
 * 链路与 GetOverlayWidgetController 完全同构，只是改取 HUD 上的 AttributesMenuController。
 *
 * 该 Controller 在 Overlay 初始化阶段就已由 AAuraHUD::InitAttributesMenu 创建并绑定回调，
 * 因此这里命中的是缓存，正常情况下不会 NewObject；属性菜单的 Widget 本身反而是在按钮点击时才懒创建。
 * 失败分支与注意事项（PC/HUD 类型不符、专用服务器、PS 未就绪）见 GetOverlayWidgetController。
 */
UAttributesMenuController* UAuraAbilitySystemLibrary::GetAttributesMenuController(const UObject* WorldContextObject)
{
	// 与 Overlay 版共用同一套「PC → HUD → PlayerState 的 ASC/AS」查找
	if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
	{
		if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAuraAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAuraAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			// InitOverlay 时已创建并 SetWidgetControllerParams 过，重复调用只是把参数再刷一遍
			return HUD->GetAttributesMenuController(WidgetControllerParams);
		}
	}
	// 同 Overlay 版：非本地端或无 Aura HUD 时返回 nullptr
	return nullptr;
}

UEnemyHealthBarWidgetController* UAuraAbilitySystemLibrary::GetElementHealthBarController(const UObject* WorldContextObject)
{
	if (const UAuraUserWidget* AuraWidget = Cast<UAuraUserWidget>(WorldContextObject))
	{
		return Cast<UEnemyHealthBarWidgetController>(AuraWidget->WidgetController);
	}
	return nullptr;
}

/**
 * 返回职业配置表。
 * 供角色初始化（属性 GE / 默认技能）时调用。
 * WorldContextObject 用来定位当前世界的 GameMode：配置挂在 GameMode 的 BP 默认值上。
 * 失败（GameMode 为空 / 未配置）时返回 nullptr：前者发生在客户端，后者的 Warning 在 GameMode 里打。
 */
const UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// 客户端没有 GameMode，GetGameMode 会返回 nullptr，这里必须判空
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	return AuraGameMode ? AuraGameMode->GetCharacterClassInfo() : nullptr;
}


bool UAuraAbilitySystemLibrary::IsDodge(const FGameplayEffectContextHandle EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* EffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->IsDodge();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* EffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->IsCriticalHit();
	}
	return false;
}

void UAuraAbilitySystemLibrary::SetIsDodge(FGameplayEffectContextHandle EffectContextHandle, const bool bIsDodge)
{
	if (FAuraGameplayEffectContext* EffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetIsDodge(bIsDodge);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle EffectContextHandle, const bool bIsCritical)
{
	if (FAuraGameplayEffectContext* EffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetIsCriticalHit(bIsCritical);
	}
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* EffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return EffectContext->GetDamageType();
	}
	return FGameplayTag();
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle EffectContextHandle, const FGameplayTag& InDamageType)
{
	if (FAuraGameplayEffectContext* EffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetDamageType(InDamageType);
	}
}

void UAuraAbilitySystemLibrary::GetLiveCharactersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const AActor* IgnoreActor, FName RequiredTag, float Radius,
	const FVector& SpherePosition)
{
	// 输出参数由本函数重置：C++ 调用方复用同一个数组时不会读到上一次查询的残留
	OutOverlappingActors.Reset();

	if (Radius <= 0.f) return;

	// 客户端/无世界上下文时 GetWorldFromContextObject 返回 nullptr，必须判空，否则其后的查询会崩溃
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return;

	// 忽略施法者自己（常见用法：AoE 不该打到释放者）；为空时不加，避免无意义条目
	FCollisionQueryParams SphereParams;
	if (IsValid(IgnoreActor))
	{
		SphereParams.AddIgnoredActor(IgnoreActor);
	}

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByObjectType(OverlapResults, SpherePosition, FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
		FCollisionShape::MakeSphere(Radius), SphereParams);

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* OverlappedActor = OverlapResult.GetActor();
		// 组件销毁/清理过程中可能拿到空 Actor，先判空再解引用
		if (!IsValid(OverlappedActor) || !OverlappedActor->Implements<UCombatInterface>())
		{
			continue;
		}
		if (ICombatInterface::Execute_IsDead(OverlappedActor))
		{
			continue;
		}
		// RequiredTag 为 None 表示不做 Tag 过滤（否则蓝图不填该引脚会永远查不到东西）
		if (!RequiredTag.IsNone() && !OverlappedActor->ActorHasTag(RequiredTag))
		{
			continue;
		}

		// 返回的是 AvatarActor：接口实现者不一定是 Actor 本身；为空时跳过，避免把空指针塞进结果
		AActor* AvatarActor = ICombatInterface::Execute_GetAvatarActor(OverlappedActor);
		if (IsValid(AvatarActor))
		{
			// 同一角色的胶囊/网格/武器会各产生一条重叠结果，AddUnique 负责去重
			OutOverlappingActors.AddUnique(AvatarActor);
		}
	}
}

/**
 * 武器挥砍轨迹检测。调用约定：攻击动画期间每帧调用一次，传入「上一帧」和「当前帧」的刀身端点。
 *
 * 原理：近战刀很快，只查当前刀身会漏判，所以要在刀身上按 SubSteps 均分出 SubSteps+1 个采样点
 * （含刀柄与刀尖）。每个采样点把「上一帧位置 → 当前帧位置」连成一条"检测段"——这就是这个点
 * 在两帧之间扫过的空间；再按调用方选定的形状（射线/球体/胶囊/盒体）对每条检测段做一次检测。
 * 一组检测段拼起来正好覆盖整把刀这一帧的挥砍轨迹，弧线挥砍、刀身平移都自然包含在内，
 * 函数内部不再单独插值刀身姿态。
 *
 * 挥拳这类"点攻击"（尖端与末端位置相同、或只配了尖端一个插槽）：
 * 刀身退化成一个点，只剩一条检测段——「上一帧拳头 → 当前帧拳头」的移动连线。
 * 球体沿连线分段采样，射线/胶囊/盒体直接用整条连线构造形状。
 * 因此同一个函数既能做刀剑的弧线扫掠，也能做拳脚的直线冲拳，调用方只改 Shape/Radius 即可。
 *
 * 命中过滤与 GetLiveCharactersWithinRadius 保持一致：
 *   - 只认 AActor + ICombatInterface + 未死亡；
 *   - 返回 GetAvatarActor 归一化后的 Actor 并去重（一个角色可能被多个采样点、多个组件各命一次）；
 *   - 本函数内部只排除 Attacker 自己；AlreadyHitActors 里的目标会被塞进碰撞查询的忽略列表，
 *     即命中过一次并加入该数组之后，后续采样点与后续帧都不再对它们做物理检测
 *     （不是"查出来再丢弃"）；额外忽略目标由调用方在结算阶段自行处理。
 *
 * 注意：本函数不施加任何伤害/GE，也不判断阵营；命中后打谁、打多少由调用方（服务器侧的技能）决定。
 */
void UAuraAbilitySystemLibrary::WeaponAttackHitDetection(
	AActor* Attacker,
	const FName WeaponBeginSocket,
	const FName WeaponEndSocket,
	const FVector& PreviousBeginLocation,
	const FVector& PreviousEndLocation,
	TArray<AActor*>& OutHitActors,
	const TArray<AActor*>& AlreadyHitActors,
	const EWeaponHitShape Shape,
	const float Radius,
	const float HalfHeight,
	const int32 SubSteps,
	const bool bDrawDebug,
	const float DebugDrawLifeTime)
{
	// 调用方通常每帧复用同一个数组，先清掉上一帧的结果，避免累计到旧命中
	OutHitActors.Reset();

	if (!IsValid(Attacker)) return;

	const UWorld* World = Attacker->GetWorld();
	if (!World) return;

	// 武器是独立骨骼网格组件（AAuraCharacterBase::Weapon），这里按插槽自动定位，调用方不需要传组件指针。
	// 优先选同时拥有两个插槽的组件；只有尖端插槽也接受（挥拳、单插槽武器），此时末端位置就等于尖端。
	USkeletalMeshComponent* WeaponMesh = nullptr;
	bool bHasEndSocket = true;
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	Attacker->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	for (USkeletalMeshComponent* SkeletalMesh : SkeletalMeshes)
	{
		if (IsValid(SkeletalMesh) && SkeletalMesh->DoesSocketExist(WeaponBeginSocket) && SkeletalMesh->DoesSocketExist(WeaponEndSocket))
		{
			WeaponMesh = SkeletalMesh;
			break;
		}
	}

	if (!WeaponMesh)
	{
		// 退一步：只有尖端插槽的骨骼网格（挥拳时通常就是手掌/拳头插槽），末端直接用尖端
		for (USkeletalMeshComponent* SkeletalMesh : SkeletalMeshes)
		{
			if (IsValid(SkeletalMesh) && SkeletalMesh->DoesSocketExist(WeaponBeginSocket))
			{
				WeaponMesh = SkeletalMesh;
				bHasEndSocket = false;
				break;
			}
		}
	}

	if (!WeaponMesh)
	{
		// Verbose：放在 Tick 里时不会刷屏；配置错了可以在 Log 里按 Verbose 过滤看到
		UE_LOG(LogTemp, Verbose, TEXT("WeaponAttackHitDetection: %s 上找不到拥有插槽 %s 的骨骼网格组件（末端插槽 %s 可选）"),
			*GetNameSafe(Attacker), *WeaponBeginSocket.ToString(), *WeaponEndSocket.ToString());
		return;
	}

	// 当前帧姿态：根部 → 尖端。尖端与末端相同时（挥拳、单插槽武器）刀身退化成一个点，
	// 整条刀身只剩一个采样点，检测段就是「上一帧点位 → 当前帧点位」的移动连线。
	const FVector CurrentBegin= WeaponMesh->GetSocketLocation(WeaponBeginSocket);
	const FVector CurrentEnd = bHasEndSocket ? WeaponMesh->GetSocketLocation(WeaponEndSocket) : CurrentBegin;

	// 上一帧姿态：全零/NaN 视为调用方尚未初始化（第一帧），退化为「只检测当前姿态」。
	// 尖端与末端分别兜底：点攻击只传尖端时，末端直接跟随尖端。
	FVector PreviousBegin = PreviousBeginLocation;
	FVector PreviousEnd = PreviousEndLocation;
	if (PreviousBegin.ContainsNaN() || PreviousBegin.IsNearlyZero())
	{
		PreviousBegin = CurrentBegin;
	}
	if (PreviousEnd.ContainsNaN() || PreviousEnd.IsNearlyZero())
	{
		PreviousEnd = PreviousBegin;
	}

	// 刀身超过 1cm 才算"刀"，在刀身上均分出采样点；否则视为点攻击（挥拳、爪击等）
	const bool bBladeMode = (CurrentBegin- CurrentEnd).Size() > 1.f;

	// 忽略攻击者自己；环境(WorldStatic)不在 AllDynamicObjects 里，本来就不会命中。
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Attacker);
	// 已经命中过的目标直接进忽略列表：它们的碰撞体不再参与后面任何一次射线/重叠查询。
	// 所以同一个敌人第一次命中并加入 AlreadyHitActors 之后，后续的采样点、后续帧都完全
	// 不再为它做物理检测，比"查出来再过滤"更省；列表为空时这一句等价于什么都不做。
	// 副作用：调试形状里被忽略的目标不会再被套住，已命中的刀段会显示成红色。
	QueryParams.AddIgnoredActors(AlreadyHitActors);
	const FCollisionObjectQueryParams ObjectParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects);

	// 形状参数：胶囊/盒体的横截面半径至少 1cm；球体允许用 <=0 表示"自动包住整段检测段"
	const float ShapeRadius = FMath::Max(Radius, 1.f);
	const float ExtraHalfLength = FMath::Max(HalfHeight, 0.f);
	const int32 StepCount = FMath::Clamp(SubSteps, 1, 16);

	// 采样点数量：刀身按 StepCount 均分，取 StepCount+1 个点（含刀柄与刀尖）；
	// 点攻击时刀身退化成一个点，只需要一个采样点。
	const int32 PointCount = bBladeMode ? StepCount : 1;

	// 调试显示：函数每帧调用，线段存活时间由调用方指定，避免几百条线叠在一起看不出单帧状态。
	// <=0 表示只画当前一帧（下一次调用就消失），由 DrawDebug 的 0 生命周期自然实现。
	const float DebugShapeLifeTime = FMath::Max(DebugDrawLifeTime, 0.f);

	// 射线与重叠的结果都走这里处理，保证过滤/去重口径一致。
	// 返回值表示这是不是一个"可命中目标"（即使它已经被本次攻击命中过），用来给检测段上色：
	// 这一段里套住任意存活敌人就是绿色，什么都没套住才是红色。
	auto ProcessHitActor = [&](AActor* HitActor) -> bool
	{
		if (!IsValid(HitActor) || HitActor == Attacker) return false;
		// 只认实现了战斗接口的对象：地板/道具/投射物等动态物体在这里被过滤掉
		if (!HitActor->Implements<UCombatInterface>()) return false;
		if (ICombatInterface::Execute_IsDead(HitActor)) return false;

		// 接口实现者不一定是 Actor 本身；返回 AvatarActor 与半径查询的结果口径一致
		AActor* AvatarActor = ICombatInterface::Execute_GetAvatarActor(HitActor);
		if (!IsValid(AvatarActor) || AvatarActor == Attacker) return false;

		// 保险判断：AlreadyHitActors 已经在上面进了查询忽略列表，正常情况下走不到这里；
		// 万一调用方在本次调用期间改了这个数组，也保证不会重复结算。
		// 这个数组是只读输入，函数不会往里写，累积由调用方 Append 完成
		if (AlreadyHitActors.Contains(AvatarActor)) return true;

		// 同一角色在多个采样点/多个碰撞组件上会重复出现，这里按 AvatarActor 去重
		if (!OutHitActors.Contains(AvatarActor))
		{
			OutHitActors.Add(AvatarActor);
		}

		return true;
	};

	// 沿一条「检测段」(AxisStart → AxisEnd) 按调用方选定的形状做一次检测。
	// 检测段 = 刀身上某个采样点在上一帧与当前帧的位置连线，也就是这个点这一帧扫过的空间。
	auto DoHitDetectionAlongAxis = [&](const FVector& AxisStart, const FVector& AxisEnd)
	{
		const FVector AxisVector = AxisEnd - AxisStart;
		const float AxisLength = AxisVector.Size();
		const bool bHasAxis = AxisLength > KINDA_SMALL_NUMBER;

		const FVector AxisDirection = bHasAxis ? AxisVector / AxisLength : FVector::ForwardVector;
		const FQuat AxisRotation = bHasAxis ? FRotationMatrix::MakeFromZ(AxisDirection).ToQuat() : FQuat::Identity;
		const FVector AxisCenter = (AxisStart + AxisEnd) * 0.5f;

		switch (Shape)
		{
		case EWeaponHitShape::Ray:
		{
			// 没有长度的射线没有意义（例如点攻击的两帧位置也相同），直接跳过
			if (!bHasAxis) break;

			TArray<FHitResult> HitResults;
			World->LineTraceMultiByObjectType(HitResults, AxisStart, AxisEnd, ObjectParams, QueryParams);

			bool bSampleHit = false;
			for (const FHitResult& HitResult : HitResults)
			{
				// 射线有真实接触点，命中点不需要额外计算；调试只看这一条段有没有套住目标
				if (ProcessHitActor(HitResult.GetActor()))
				{
					bSampleHit = true;
				}
			}

			if (bDrawDebug)
			{
				// 红 = 这条射线没碰到目标；绿 = 碰到了
				DrawDebugLine(World, AxisStart, AxisEnd, bSampleHit ? FColor::Green : FColor::Red, false, DebugShapeLifeTime, 0, 1.f);
			}
			break;
		}

		case EWeaponHitShape::Sphere:
		{
			// Radius<=0 时自动取轴半长，把整段轴包进一个球；轴退化成一个点时保底 1cm
			const float SphereRadius = Radius > 0.f ? Radius : FMath::Max(AxisLength * 0.5f, 1.f);
			const FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);

			TArray<FOverlapResult> OverlapResults;
			World->OverlapMultiByObjectType(OverlapResults, AxisCenter, FQuat::Identity, ObjectParams, SphereShape, QueryParams);

			bool bSampleHit = false;
			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				if (ProcessHitActor(OverlapResult.GetActor()))
				{
					bSampleHit = true;
				}
			}

			if (bDrawDebug)
			{
				// 红 = 这个球没碰到目标；绿 = 碰到了
				DrawDebugSphere(World, AxisCenter, SphereRadius, 8, bSampleHit ? FColor::Green : FColor::Red, false, DebugShapeLifeTime, 0, 1.f);
			}
			break;
		}

		case EWeaponHitShape::Capsule:
		{
			// 圆柱段覆盖整条轴，两端半球各多出 Radius；HalfHeight 再额外加长
			const float CapsuleHalfHeight = FMath::Max(AxisLength * 0.5f + ShapeRadius + ExtraHalfLength, ShapeRadius);
			const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(ShapeRadius, CapsuleHalfHeight);

			TArray<FOverlapResult> OverlapResults;
			World->OverlapMultiByObjectType(OverlapResults, AxisCenter, AxisRotation, ObjectParams, CapsuleShape, QueryParams);

			bool bSampleHit = false;
			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				if (ProcessHitActor(OverlapResult.GetActor()))
				{
					bSampleHit = true;
				}
			}

			if (bDrawDebug)
			{
				// 红 = 这个胶囊没碰到目标；绿 = 碰到了
				DrawDebugCapsule(World, AxisCenter, CapsuleHalfHeight, ShapeRadius, AxisRotation, bSampleHit ? FColor::Green : FColor::Red, false, DebugShapeLifeTime, 0, 1.f);
			}
			break;
		}

		case EWeaponHitShape::Box:
		default:
		{
			// 盒体的 Z 半长覆盖整条轴，X/Y 半尺寸由 Radius 决定；HalfHeight 再额外加长
			const FVector BoxExtent(ShapeRadius, ShapeRadius, FMath::Max(AxisLength * 0.5f + ExtraHalfLength, 1.f));
			const FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);

			TArray<FOverlapResult> OverlapResults;
			World->OverlapMultiByObjectType(OverlapResults, AxisCenter, AxisRotation, ObjectParams, BoxShape, QueryParams);

			bool bSampleHit = false;
			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				if (ProcessHitActor(OverlapResult.GetActor()))
				{
					bSampleHit = true;
				}
			}

			if (bDrawDebug)
			{
				// 红 = 这个盒体没碰到目标；绿 = 碰到了
				DrawDebugBox(World, AxisCenter, BoxExtent, AxisRotation, bSampleHit ? FColor::Green : FColor::Red, false, DebugShapeLifeTime, 0, 1.f);
			}
			break;
		}
		}
	};

	// 在刀身上均分采样点：每个采样点用自己「上一帧 → 当前帧」的位移作为一条检测段。
	// 弧线挥砍、刀身平移都自然落在这些检测段里，函数内部不再单独插值刀身姿态。
	for (int32 PointIndex = 0; PointIndex <= PointCount; ++PointIndex)
	{
		// 0 = 刀柄（末端），1 = 刀尖；点攻击时两端重合，只有一个采样点
		const float Alpha = PointCount > 1 ? static_cast<float>(PointIndex) / static_cast<float>(PointCount - 1) : 0.f;

		// 同一个采样点在上一帧与当前帧的位置，这两点之间就是它这一帧扫过的空间
		const FVector PreviousPoint = FMath::Lerp(PreviousEnd, PreviousBegin, Alpha);
		const FVector CurrentPoint = FMath::Lerp(CurrentEnd, CurrentBegin, Alpha);

		DoHitDetectionAlongAxis(PreviousPoint, CurrentPoint);
	}

	// 说明：每个采样点、每一帧都独立检测，重复命中的目标统一由 AlreadyHitActors 拦截，
	// 所以一次攻击可以命中多个不同敌人，但同一个敌人只会结算一次。
}