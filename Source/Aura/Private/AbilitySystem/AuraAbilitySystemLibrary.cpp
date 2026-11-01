// Study Game Ability System _Hades

/**
 * @file AuraAbilitySystemLibrary.cpp
 * @brief 全局查询库实现：职业配置表 / 角色名单的取用，以及本地玩家的 UI WidgetController 获取。
 *
 * 要点：
 *  - 四个函数都是「查一下、返回指针」，不改任何状态，因此不需要复制、不需要广播，也没有服务器限制；
 *  - 配置类函数把数据源固定为 UAuraGameSettings（Project Settings 里的进程级配置），
 *    这样菜单关、战斗关、客户端读到的是同一份资产，不随关卡切换丢失；
 *  - UI 类函数的链路是「0 号 PC → HUD → 组装 FWidgetControllerParams → HUD 的懒加载单例」，
 *    只在有本地玩家的端有效，专用服务器上返回 nullptr。
 */

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Game/AuraGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"

/**
 * 返回职业配置表。
 * 供角色初始化（属性 GE / 默认技能）、角色定义资产查询职业信息时调用。
 * WorldContextObject 在实现里完全没用——保留它只是为了蓝图的 World Context 自动注入，
 * 配置本身挂在 UAuraGameSettings 的 CDO 上（Project Settings → Game → Aura Game Settings），
 * 与当前是哪个世界、哪个 GameMode 无关。所以这里的签名参数被刻意忽略，不是遗漏。
 * 失败（未配置）时由 UAuraGameSettings 打 Warning 并返回 nullptr。
 */
const UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// 唯一读取点：数据源在 Project Settings，而不是某个关卡的 GameMode。
	// WorldContextObject 只服务 BP 的隐式上下文（meta = WorldContext），配置本身是进程级的。
	return UAuraGameSettings::Get()->GetCharacterClassInfo();
}

/**
 * 返回可选角色名单。
 * 与 GetCharacterClassInfo 同一套机制（CDO + 同步加载），但更"轻"：未配置时静默返回 nullptr，
 * 因为名单只服务选人界面，缺了不影响战斗流程。
 */
const UAuraCharacterRoster* UAuraAbilitySystemLibrary::GetCharacterRoster(const UObject* WorldContextObject)
{
	return UAuraGameSettings::Get()->GetCharacterRoster();
}

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
