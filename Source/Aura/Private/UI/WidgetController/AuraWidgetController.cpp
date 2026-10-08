// Study Game Ability System _Hades

#include "UI/WidgetController/AuraWidgetController.h"

void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams)
{
	// 将 FWidgetControllerParams 中的四个核心 GAS 引用复制到成员变量
	AuraPlayerController = WidgetControllerParams.AuraPlayerController;
	AuraPlayerState = WidgetControllerParams.AuraPlayerState;
	AuraAbilitySystemComponent = WidgetControllerParams.AuraAbilitySystemComponent;
	AuraAttributeSet = WidgetControllerParams.AuraAttributeSet;
}

void UAuraWidgetController::BroadCastInitialValues()
{
	// 基类空实现：子类必须重写以广播各自的初始属性值
	// 例如 OverlayWidgetController 重写此方法广播 Health 和 MaxHealth
}

void UAuraWidgetController::BindCallbacksToDependencies()
{
}
