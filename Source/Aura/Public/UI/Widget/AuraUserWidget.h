// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * Aura 用户 Widget 基类
 *
 * 持有 WidgetController 引用作为数据源，并提供 WidgetControllerSet 蓝图事件，
 * 供子 Widget 在蓝图中接收数据绑定通知。
 *
 * 数据流向（树形传播，非递归）：
 *   HUD 调用父 Widget 的 SetWidgetController(OverlayController)
 *     → C++ 赋值并触发 WidgetControllerSet 蓝图事件
 *       → 蓝图中，父 Widget 的 WidgetControllerSet 调用子 Widget（HealthGlobe、ManaGlobe）的 SetWidgetController
 *         → 每个子 Widget 同样赋值并触发自己的 WidgetControllerSet
 *           → 终止（子 Widget 的蓝图实现不再向上/向外传播）
 *
 * 不会死循环，因为：
 *   - 调用的是不同对象（父 → 子），不是 self 递归
 *   - 传播方向单向（父 → 子），子 Widget 不会回调用父 Widget
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * 设置 WidgetController 并触发 WidgetControllerSet 蓝图事件
	 *
	 * 注意：此函数被 C++ 和蓝图双重调用：
	 *   - C++ 侧：HUD::InitOverlay() 给根 Widget 设置 OverlayWidgetController
	 *   - 蓝图侧：父 Widget 的 WidgetControllerSet 事件中，将同一 Controller 传给子 Widget
	 */
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);

	/** 当前绑定的 WidgetController（只读，供蓝图绑定回调使用） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UObject> WidgetController;

protected:
	/**
	 * WidgetController 设置完成后的蓝图事件
	 *
	 * 在蓝图中重写此事件，将 WidgetController 分发给所有子 Widget。
	 * 例如：WBP_AuraHUD 的 WidgetControllerSet 中调用 HealthGlobe->SetWidgetController(...)
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
