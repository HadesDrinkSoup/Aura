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
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
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