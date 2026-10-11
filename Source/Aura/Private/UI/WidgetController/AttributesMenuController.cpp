// Study Game Ability System _Hades


#include "UI/WidgetController/AttributesMenuController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributesInfo.h"
#include "UI/WidgetController/OverlayWidgetController.h"

void UAttributesMenuController::BroadCastInitialValues()
{
	// 防御：数据资产未配置时给出明确错误，避免空指针崩溃
	if (!AttributeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("[AttributesMenuController] AttributeInfo 数据资产未配置，请在 Bp_AttributesMenuController 的 Attributes Information -> Attribute Info 中赋值"));
		return;
	}

	for (const TTuple<FGameplayTag, TDelegate<FGameplayAttribute()>>& Pair : AuraAttributeSet->TagsToAttributesMap)
	{
		BroadcastAttributeInfo(Pair.Key);
	}
}

void UAttributesMenuController::BindCallbacksToDependencies()
{
	if (bCallbacksBound) return;
	for (TTuple<FGameplayTag, TDelegate<FGameplayAttribute()>>& Pair : AuraAttributeSet->TagsToAttributesMap)
	{
		AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value.Execute()).AddLambda(
			[this, AttributeTag = Pair.Key](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(AttributeTag);
			});
	}
	bCallbacksBound = true;
}

void UAttributesMenuController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag) const
{
	if (!AttributeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("[AttributesMenuController] AttributeInfo 数据资产未配置，请在 Bp"
							  ""
		 "_AttributesMenuController 的 Attributes Information -> Attribute Info 中赋值"));
		return;
	}

	FAuraAttributeInfo* Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag, false);
	if (!Info)
	{
		// 该属性标签在数据资产中缺少对应条目：记录并跳过，避免空指针崩溃
		UE_LOG(LogTemp, Warning, TEXT("[AttributesMenuController] DA_AttributesInfo 中缺少标签 [%s] 的条目，已跳过该属性"), *AttributeTag.ToString());
		return;
	}

	const FAttributeDelegate* AttributeDelegate = AuraAttributeSet->TagsToAttributesMap.Find(AttributeTag);
	if (!AttributeDelegate)
	{
		return;
	}

	Info->AttributeValue = AttributeDelegate->Execute().GetNumericValue(AuraAttributeSet);
	AttributeInfoChangedDelegate.Broadcast(*Info);
}
