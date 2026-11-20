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
#include "Engine/SkeletalMesh.h"
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

void UAuraAbilitySystemLibrary::SetElementType(FGameplayEffectContextHandle EffectContextHandle, const FGameplayTag& InElementType)
{
	if (FAuraGameplayEffectContext* EffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		EffectContext->SetDamageType(InElementType);
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

namespace
{
    /** 某把武器上一次采样时的刀身姿态；LastSampleTime < 0 表示还没有历史 */
    struct FWeaponSweepHistory
    {
        double LastSampleTime = -1.0;
        FVector BeginLocation = FVector::ZeroVector;
        FVector EndLocation = FVector::ZeroVector;
    };

    /** 跨帧缓存键：一个攻击者的一把武器，由起止插槽名共同标识 */
    struct FWeaponSweepEntry
    {
        TWeakObjectPtr<AActor> Attacker;
        FName BeginSocket;
        FName EndSocket;
        FWeaponSweepHistory History;
    };

    /** 上一帧刀身姿态的进程级缓存，只在游戏线程访问 */
    TArray<FWeaponSweepEntry> WeaponSweepEntries;

    FWeaponSweepHistory& FindOrAddWeaponSweepHistory(AActor* Attacker, const FName BeginSocket, const FName EndSocket)
    {
        WeaponSweepEntries.RemoveAll([](const FWeaponSweepEntry& Entry) { return !Entry.Attacker.IsValid(); });

        for (FWeaponSweepEntry& Entry : WeaponSweepEntries)
        {
            if (Entry.Attacker.Get() == Attacker && Entry.BeginSocket == BeginSocket && Entry.EndSocket == EndSocket)
                return Entry.History;
        }

        FWeaponSweepEntry& NewEntry = WeaponSweepEntries.AddDefaulted_GetRef();
        NewEntry.Attacker = Attacker;
        NewEntry.BeginSocket = BeginSocket;
        NewEntry.EndSocket = EndSocket;
        return NewEntry.History;
    }

    /** 优先返回同时有「起点 + 终点」插槽的网格；找不到再退回只有起点插槽的（挥拳、单插槽武器） */
    USkeletalMeshComponent* FindWeaponMesh(const AActor* Attacker, const FName BeginSocket, const FName EndSocket)
    {
        TArray<USkeletalMeshComponent*> Meshes;
        Attacker->GetComponents<USkeletalMeshComponent>(Meshes);

        USkeletalMeshComponent* SingleSocketMesh = nullptr;
        for (USkeletalMeshComponent* Mesh : Meshes)
        {
            if (!IsValid(Mesh) || !Mesh->DoesSocketExist(BeginSocket)) continue;
            if (Mesh->DoesSocketExist(EndSocket)) return Mesh;
            if (!SingleSocketMesh) SingleSocketMesh = Mesh;
        }
        return SingleSocketMesh;
    }

    /** 单插槽时用包围盒最长轴近似刀身方向（武器是细长形）；无资产返回零向量 */
    FVector FindWeaponAxis(const USkeletalMeshComponent* WeaponMesh)
    {
        const USkeletalMesh* MeshAsset = WeaponMesh ? WeaponMesh->GetSkeletalMeshAsset() : nullptr;
        if (!MeshAsset) return FVector::ZeroVector;

        const FVector Extent = MeshAsset->GetBounds().BoxExtent;
        int32 LongestAxis = 0;
        if (Extent.Y > Extent[LongestAxis]) LongestAxis = 1;
        if (Extent.Z > Extent[LongestAxis]) LongestAxis = 2;

        FVector LocalAxis = FVector::ZeroVector;
        LocalAxis[LongestAxis] = 1.f;
        return WeaponMesh->GetComponentQuat().RotateVector(LocalAxis).GetSafeNormal();
    }

    /** 归一化命中 Actor 为 AvatarActor；无效、自身、非战斗接口、已死亡返回 nullptr */
    AActor* ResolveHitTarget(AActor* HitActor, const AActor* Attacker)
    {
        if (!IsValid(HitActor) || HitActor == Attacker) return nullptr;
        if (!HitActor->Implements<UCombatInterface>() || ICombatInterface::Execute_IsDead(HitActor)) return nullptr;

        AActor* AvatarActor = ICombatInterface::Execute_GetAvatarActor(HitActor);
        return (IsValid(AvatarActor) && AvatarActor != Attacker) ? AvatarActor : nullptr;
    }

    /** 一条检测段的几何：两端、中心、朝向，以及按 Shape 解释出的碰撞形状 */
    struct FSegmentShape
    {
        FVector Start = FVector::ZeroVector;
        FVector End = FVector::ZeroVector;
        FVector Center = FVector::ZeroVector;
        FQuat Rotation = FQuat::Identity;
        bool bHasLength = false;

        FCollisionShape CollisionShape = FCollisionShape::MakeSphere(1.f); // Ray 不使用
        float Radius = 0.f;
        float HalfHeight = 0.f;
        FVector Extent = FVector::ZeroVector;
    };

    FSegmentShape MakeSegmentShape(const EWeaponHitShape Shape, const FVector& Start, const FVector& End,
        const float Radius, const float HalfHeight, const FVector& CapsuleAxis = FVector::ZeroVector)
    {
        FSegmentShape Segment;
        Segment.Start = Start;
        Segment.End = End;
        Segment.Center = (Start + End) * 0.5f;

        const FVector SegmentVector = End - Start;
        const float SegmentLength = SegmentVector.Size();
        Segment.bHasLength = SegmentLength > KINDA_SMALL_NUMBER;

        if (Shape == EWeaponHitShape::Ray) return Segment;

        if (Segment.bHasLength)
            Segment.Rotation = FRotationMatrix::MakeFromZ(SegmentVector / SegmentLength).ToQuat();

        const float SafeRadius  = FMath::Max(Radius, 1.f);        // 非球体形状：Radius <= 0 按 1cm
        const float ExtraHalf   = FMath::Max(HalfHeight, 0.f);    // 胶囊/盒体在段半长之外再延伸
        const float HalfLen     = SegmentLength * 0.5f;

        switch (Shape)
        {
        case EWeaponHitShape::Sphere:
            // Radius <= 0 表示把整条检测段包进一个球；球没有朝向
            Segment.Radius = Radius > 0.f ? Radius : FMath::Max(HalfLen, 1.f);
            Segment.CollisionShape = FCollisionShape::MakeSphere(Segment.Radius);
            Segment.Rotation = FQuat::Identity;
            break;

        case EWeaponHitShape::Capsule:
            Segment.Radius = SafeRadius;
            Segment.HalfHeight = FMath::Max(HalfLen + SafeRadius + ExtraHalf, SafeRadius);
            Segment.CollisionShape = FCollisionShape::MakeCapsule(Segment.Radius, Segment.HalfHeight);
            // 胶囊平躺在刀身上：长轴沿刀身，而不是沿检测段
            if (!CapsuleAxis.IsNearlyZero())
                Segment.Rotation = FRotationMatrix::MakeFromZ(CapsuleAxis.GetSafeNormal()).ToQuat();
            break;

        case EWeaponHitShape::Box:
        default:
            Segment.Radius = SafeRadius;
            Segment.Extent = FVector(SafeRadius, SafeRadius, FMath::Max(HalfLen + ExtraHalf, 1.f));
            Segment.CollisionShape = FCollisionShape::MakeBox(Segment.Extent);
            break;
        }
        return Segment;
    }

    /** 按所选形状绘制一次检测：命中画绿，否则画红 */
    void DrawSegmentShape(const UWorld* World, const FSegmentShape& Segment, const EWeaponHitShape Shape,
        const bool bHit, const float LifeTime)
    {
        const FColor Color = bHit ? FColor::Green : FColor::Red;
        switch (Shape)
        {
        case EWeaponHitShape::Ray:     DrawDebugLine(World, Segment.Start, Segment.End, Color, false, LifeTime, 0, 1.f); break;
        case EWeaponHitShape::Sphere:  DrawDebugSphere(World, Segment.Center, Segment.Radius, 8, Color, false, LifeTime, 0, 1.f); break;
        case EWeaponHitShape::Capsule: DrawDebugCapsule(World, Segment.Center, Segment.HalfHeight, Segment.Radius, Segment.Rotation, Color, false, LifeTime, 0, 1.f); break;
        case EWeaponHitShape::Box:
        default:                       DrawDebugBox(World, Segment.Center, Segment.Extent, Segment.Rotation, Color, false, LifeTime, 0, 1.f); break;
        }
    }

    /** 对一条检测段做一次物理查询，命中结果归一化去重后追加到 OutHitActors */
    bool SweepSegment(
        const UWorld* World,
        const FCollisionQueryParams& QueryParams,
        const FCollisionObjectQueryParams& ObjectParams,
        const AActor* Attacker,
        const EWeaponHitShape Shape,
        const FVector& SegmentStart,
        const FVector& SegmentEnd,
        const FVector& CapsuleAxis,
        const float Radius,
        const float HalfHeight,
        const bool bDrawDebug,
        const float DebugLifeTime,
        TArray<AActor*>& OutHitActors)
    {
        const FSegmentShape Segment = MakeSegmentShape(Shape, SegmentStart, SegmentEnd, Radius, HalfHeight, CapsuleAxis);
        // 射线没有体积：该采样点两帧没动时直接跳过
        if (Shape == EWeaponHitShape::Ray && !Segment.bHasLength) return false;

        // 先用一个容器统一收集原始命中 Actor，射线/重叠只在查询调用上不同
        TArray<AActor*> RawHitActors;
        if (Shape == EWeaponHitShape::Ray)
        {
            TArray<FHitResult> Results;
            World->LineTraceMultiByObjectType(Results, Segment.Start, Segment.End, ObjectParams, QueryParams);
            for (const FHitResult& R : Results) RawHitActors.Add(R.GetActor());
        }
        else
        {
            TArray<FOverlapResult> Results;
            World->OverlapMultiByObjectType(Results, Segment.Center, Segment.Rotation, ObjectParams, Segment.CollisionShape, QueryParams);
            for (const FOverlapResult& R : Results) RawHitActors.Add(R.GetActor());
        }

        bool bHit = false;
        for (AActor* Raw : RawHitActors)
        {
            if (AActor* Target = ResolveHitTarget(Raw, Attacker))
            {
                bHit = true;
                OutHitActors.AddUnique(Target);
            }
        }

        if (bDrawDebug) DrawSegmentShape(World, Segment, Shape, bHit, DebugLifeTime);
        return bHit;
    }
}

void UAuraAbilitySystemLibrary::WeaponAttackHitDetection(
    AActor* Attacker,
    const FName WeaponBeginSocket,
    const FName WeaponEndSocket,
    TArray<AActor*>& OutHitActors,
    const TArray<AActor*>& AlreadyHitActors,
    const EWeaponHitShape Shape,
    const float Radius,
    const float HalfHeight,
    const int32 SubSteps,
    const bool bDrawDebug,
    const float DebugDrawLifeTime)
{
    OutHitActors.Reset();
    if (!IsValid(Attacker)) return;
    const UWorld* World = Attacker->GetWorld();
    if (!World) return;

    const USkeletalMeshComponent* WeaponMesh = FindWeaponMesh(Attacker, WeaponBeginSocket, WeaponEndSocket);
    if (!WeaponMesh) return;

    // 当前姿态：刀身两端。单插槽/挥拳时两端重合，后面会退化成点检测
    const FVector CurrentBegin = WeaponMesh->GetSocketLocation(WeaponBeginSocket);
    const FVector CurrentEnd = WeaponMesh->DoesSocketExist(WeaponEndSocket)
        ? WeaponMesh->GetSocketLocation(WeaponEndSocket)
        : CurrentBegin;

    // 上一帧姿态：第一次调用还没有历史，当帧退化为只检测当前姿态
    const double CurrentTime = World->GetTimeSeconds();
    FWeaponSweepHistory& History = FindOrAddWeaponSweepHistory(Attacker, WeaponBeginSocket, WeaponEndSocket);
    const bool bContinueSwing = History.LastSampleTime >= 0.0;
    const FVector PreviousBegin = bContinueSwing ? History.BeginLocation : CurrentBegin;
    const FVector PreviousEnd   = bContinueSwing ? History.EndLocation   : CurrentEnd;

    // 忽略自身以及本次攻击已命中的目标
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Attacker);
    QueryParams.AddIgnoredActors(AlreadyHitActors);
    const FCollisionObjectQueryParams ObjectParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects);

    // 沿刀身均分采样：每个采样点检测「上一帧位置 → 当前帧位置」；单插槽时退化为单点
    const FVector BladeVector  = CurrentBegin - CurrentEnd;
    const float   BladeLength  = BladeVector.Size();
    const bool    bBladeMode   = BladeLength > 1.f;
    const int32   SampleCount  = bBladeMode ? FMath::Clamp(SubSteps, 1, 32) : 0;
    const FVector BladeAxis    = bBladeMode ? BladeVector / BladeLength : FindWeaponAxis(WeaponMesh);
    const float   DebugLifeTime = FMath::Max(DebugDrawLifeTime, 0.f);

    for (int32 i = 0; i <= SampleCount; ++i)
    {
        const float Alpha = SampleCount > 0 ? static_cast<float>(i) / SampleCount : 0.f;
        SweepSegment(
            World, QueryParams, ObjectParams, Attacker, Shape,
            FMath::Lerp(PreviousBegin, PreviousEnd, Alpha),
            FMath::Lerp(CurrentBegin,  CurrentEnd,  Alpha),
            BladeAxis, Radius, HalfHeight, bDrawDebug, DebugLifeTime, OutHitActors);
    }

    // 缓存本帧姿态，作为下一次调用的"上一帧"
    History.LastSampleTime = CurrentTime;
    History.BeginLocation  = CurrentBegin;
    History.EndLocation    = CurrentEnd;
}