// Study Game Ability System _Hades
//
// @file  AuraInputConfig.cpp
// @brief 输入配置数据资产的实现：按 GameplayTag 反向查找输入动作。

#include "Input/AuraInputConfig.h"

/**
 * 按 GameplayTag 顺序查找对应的输入动作。
 * 调用方：需要"由标签反查按键"的场景（例如按键提示 UI）。
 * 主输入链路（BindAbilityActions）走的是正向遍历绑定，不经过本函数，
 * 而列表只有几条到十几条，因此这里的 O(n) 遍历不构成性能问题。
 * 未找到时若 bLogFound 为真，会以 Error 级别打日志，方便定位"配置漏填"。
 */
const UInputAction* UAuraInputConfig::FindInputActionForTag(const FGameplayTag& InputTag, bool bLogFound) const
{
	// 逐条比对：跳过没有配置输入动作的条目，标签完全相等即命中
	for (const FAuraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	// 遍历结束仍未命中：按需要给出错误日志（调用方通常无法自己判断是"没配"还是"配错"）
	if (bLogFound)
	{
		UE_LOG(LogTemp, Error, TEXT("在InputConfig [%s] 中缺少输入标签 [%s]"), *GetNameSafe(this), *InputTag.ToString());
	}
	// 返回空指针，调用方必须判空
	return nullptr;
}
