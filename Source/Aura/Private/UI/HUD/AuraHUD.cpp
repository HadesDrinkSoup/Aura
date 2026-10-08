// Study Game Ability System _Hades

#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AttributesMenuController.h"
#include "UI/WidgetController/OverlayWidgetController.h"

void AAuraHUD::InitOverlay(const FWidgetControllerParams& WidgetControllerParams)
{
	// 确保蓝图已配置 Widget 类和 Controller 类，否则崩溃并给出明确错误信息
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass未初始化，请在Bp_AuraHUD设置"));
	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass未初始化，请在Bp_AuraHUD设置"));

	// 防重复：Overlay 只初始化一次
	if (OverlayWidget && OverlayWidget->IsInViewport()) return;

	// 1. 创建 Overlay Widget 实例
	UAuraUserWidget* Overlay = CreateWidget<UAuraUserWidget>(GetWorld(), OverlayWidgetClass);

	// 2. 获取（或创建）WidgetController，传入 GAS 四大核心依赖
	UOverlayWidgetController* WidgetControllerOverlay = GetOverlayWidgetController(WidgetControllerParams);


	// 3. 将 WidgetController 绑定到 Widget
	//    SetWidgetController 内部会触发 WidgetControllerSet 蓝图事件，
	//    蓝图中通过该事件将 Controller 继续下发给子 Widget（如 HealthGlobe、ManaGlobe）
	Overlay->SetWidgetController(WidgetControllerOverlay);

	// 4. 绑定属性变化回调（内置防重复），必须在广播初始值之前
	WidgetControllerOverlay->BindCallbacksToDependencies();

	// 5. 广播初始属性值，让 Widget 显示正确的初始数据
	WidgetControllerOverlay->BroadCastInitialValues();

	// 6. 添加到视口显示
	Overlay->AddToViewport();

	// 7. 提前绑定属性菜单回调（Widget 由蓝图按钮点击时延迟创建）
	InitAttributesMenu(WidgetControllerParams);
}

void AAuraHUD::InitAttributesMenu(const FWidgetControllerParams& WidgetControllerParams)
{
	AttributesMenuController = GetAttributesMenuController(WidgetControllerParams);
	AttributesMenuController->BindCallbacksToDependencies();
}

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WidgetControllerParams)
{
	// 懒加载单例：已存在则直接返回缓存
	if (OverlayWidgetController) return OverlayWidgetController;

	// 首次调用时创建，Outer 设为 this（HUD），生命周期由 HUD 管理
	OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
	OverlayWidgetController->SetWidgetControllerParams(WidgetControllerParams);
	return OverlayWidgetController;
}

UAttributesMenuController* AAuraHUD::GetAttributesMenuController(const FWidgetControllerParams& WidgetControllerParams)
{
	// 懒加载单例：已存在则直接返回缓存
	if (AttributesMenuController) return AttributesMenuController;

	// 首次调用时创建，Outer 设为 this（HUD），生命周期由 HUD 管理
	AttributesMenuController = NewObject<UAttributesMenuController>(this, AttributesMenuControllerClass);
	AttributesMenuController->SetWidgetControllerParams(WidgetControllerParams);
	return AttributesMenuController;
}
