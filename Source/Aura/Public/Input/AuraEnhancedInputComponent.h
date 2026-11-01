// Study Game Ability System _Hades
// 增强型输入组件：继承 UEnhancedInputComponent，
// 负责根据输入配置（UAuraInputConfig）批量绑定能力输入，
// 并在触发时把对应的 GameplayTag 透传给回调函数。

#pragma once

#include "CoreMinimal.h"
#include "AuraInputConfig.h"
#include "EnhancedInputComponent.h"
#include "AuraEnhancedInputComponent.generated.h"

struct FAuraInputAction;
class UAuraInputConfig;

/**
 * Aura 的自定义输入组件，扩展了 UEnhancedInputComponent。
 * 作用：把配置里的一组「输入动作 ↔ GameplayTag」映射，
 * 一次性绑定到「按下 / 松开 / 按住」三种触发事件上，
 * 触发回调时携带对应的 GameplayTag。
 *
 * 使用位置：AAuraPlayerController::SetupInputComponent 中把 Cast 后的本组件
 * 与 AuraInputConfig 一起传入 BindAbilityActions，回调各有三个：
 * AbilityInputTagPressed / Released / Held。
 */
UCLASS()
class AURA_API UAuraEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	/**
	 * 批量绑定能力输入。
	 * 遍历 InputConfig 中的所有输入动作映射，分别为其绑定：
	 *   - PressedFunc  → ETriggerEvent::Started（按下）
	 *   - ReleasedFunc → ETriggerEvent::Completed（松开）
	 *   - HeldFunc     → ETriggerEvent::Triggered（按住）
	 * 每个回调都会携带对应的 GameplayTag（Action.InputTag）作为参数。
	 *
	 * 实现要点：UEnhancedInputComponent::BindAction 支持在委托参数之后追加
	 * 额外的透传参数，这里把 Action.InputTag 作为追加参数传入，于是回调签名
	 * 可以写成 (const FInputActionValue& Value, FGameplayTag InputTag)，
	 * 无需在回调里反查是哪个输入触发了它。
	 *
	 * @tparam UserClass       持有回调函数的对象类型（通常是角色或玩家控制器）
	 * @tparam PressedFuncType 按下回调的函数类型
	 * @tparam ReleasedFuncType 松开回调的函数类型
	 * @tparam HeldFuncType    按住回调的函数类型
	 * @param InputConfig  输入配置数据资产，提供要绑定的映射列表
	 * @param Object       回调函数所属的对象实例
	 * @param PressedFunc  按下触发时的回调（可为空）
	 * @param ReleasedFunc 松开触发时的回调（可为空）
	 * @param HeldFunc     按住触发时的回调（可为空）
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};

/**
 * BindAbilityActions 的模板实现（模板函数需在头文件内定义）。
 * 逐个检查映射：仅当输入动作与 GameplayTag 均有效时才绑定，
 * 且仅当传入的对应回调非空时才为该事件绑定，避免无效绑定。
 */
template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UAuraEnhancedInputComponent::BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object, 
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	// 输入配置必须有效
	check(InputConfig);
	for (const FAuraInputAction& Action : InputConfig->AbilityInputActions)
	{
		// 只有输入动作和 GameplayTag 都有效时才进行绑定
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				// 按下（Started）事件
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}
			if (ReleasedFunc)
			{
				// 松开（Completed）事件
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}
			if (HeldFunc)
			{
				// 按住（Triggered）事件
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
