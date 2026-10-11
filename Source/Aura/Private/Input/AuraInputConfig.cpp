// Study Game Ability System _Hades

#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindInputActionForTag(const FGameplayTag& InputTag, bool bLogFound) const
{
	for (const FAuraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	if (bLogFound)
	{
		UE_LOG(LogTemp, Error, TEXT("在InputConfig [%s] 中缺少输入标签 [%s]"), *GetNameSafe(this), *InputTag.ToString());
	}
	return nullptr;
}
