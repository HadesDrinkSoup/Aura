// Study Game Ability System _Hades

#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* InWidgetController)
{
	// 1. 保存 WidgetController 引用，供蓝图通过绑定访问
	WidgetController = InWidgetController;

	// 2. 触发蓝图事件 WidgetControllerSet
	//    蓝图中通过此事件将 Controller 继续传递给子 Widget，
	//    形成单向树形数据传播（父 → 子，不会死循环）
	WidgetControllerSet();
}
