// Study Game Ability System _Hades


#include "UI/WidgetController/AttributesMenuController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributesInfo.h"

void UAttributesMenuController::BroadCastInitialValues()
{
	// 防御：数据资产未配置时给出明确错误，避免空指针崩溃
	if (!AttributeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("[AttributesMenuController] AttributeInfo 数据资产未配置，请在 Bp_AttributesMenuController 的 Attributes Information -> Attribute Info 中赋值"));
		return;
	}

	for (const TTuple<FGameplayTag, TDelegate<FGameplayAttribute()>>& Pair : AuraAttributeSet->TagToAttributeMap)
	{
		FAuraAttributeInfo* Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key, false);
		if (!Info)
		{
			// 该属性标签在数据资产中缺少对应条目：记录并跳过，避免空指针崩溃
			UE_LOG(LogTemp, Warning, TEXT("[AttributesMenuController] DA_AttributesInfo 中缺少标签 [%s] 的条目，已跳过该属性"), *Pair.Key.ToString());
			continue;
		}
		Info->AttributeValue = Pair.Value.Execute().GetNumericValue(AuraAttributeSet);
		AttributeInfoChangedDelegate.Broadcast(*Info);
	}
}

void UAttributesMenuController::BindCallbacksToDependencies()
{
	if (bCallbacksBound) return;
	
	bCallbacksBound = true;
}
