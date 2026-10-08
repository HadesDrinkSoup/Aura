// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

class UAttributesMenuController;
struct FWidgetControllerParams;
class UOverlayWidgetController;
class UAuraUserWidget;

/**
 * Aura 主 HUD
 *
 * 负责 Overlay UI 的创建与初始化流程：
 *   1. InitOverlay() 被调用时，创建 OverlayWidget（蓝图 Widget 实例）
 *   2. 获取/创建 OverlayWidgetController，传入 GAS 依赖（PC, PS, ASC, AS）
 *   3. 调用 Widget->SetWidgetController() 绑定数据源
 *   4. 广播初始值，将 Widget 添加到视口
 *
 * OverlayWidgetController 采用懒加载单例模式，同一 HUD 生命周期内只创建一个。
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:
	/** 初始化主 Overlay UI：创建 Widget + WidgetController，绑定并显示 */
	void InitOverlay(const FWidgetControllerParams& WidgetControllerParams);

	/** 初始化属性菜单 Controller 并绑定回调（Widget 由蓝图按钮点击时延迟创建） */
	void InitAttributesMenu(const FWidgetControllerParams& WidgetControllerParams);

	/** 获取 OverlayWidgetController（懒加载，首次调用时创建并缓存） */
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WidgetControllerParams);
	UAttributesMenuController* GetAttributesMenuController(const FWidgetControllerParams& WidgetControllerParams);

private:
	/** 缓存的 OverlayWidget 实例指针 */
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	/** 缓存的 OverlayWidgetController 实例指针（懒加载） */
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	/** 蓝图 OverlayWidget 子类（在 BP_AuraHUD 中配置） */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;
	
	/** 蓝图 OverlayWidgetController 子类（在 BP_AuraHUD 中配置） */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
	
	UPROPERTY()
	TObjectPtr<UAttributesMenuController> AttributesMenuController;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAuraUserWidget> AttributesMenuClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributesMenuController> AttributesMenuControllerClass;
};
