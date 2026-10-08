// Study Game Ability System _Hades


#include "UI/WidgetController/AttributesMenuController.h"
#include "AuraGamepalyTags.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributesInfo.h"
#include "Engine/RendererSettings.h"

void UAttributesMenuController::BroadCastInitialValues()
{
	for (const TTuple<FGameplayTag, TDelegate<FGameplayAttribute()>>& Pair : AuraAttributeSet->TagToAttributeMap)
	{
		FAuraAttributeInfo* Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
		Info->AttributeValue = Pair.Value.Execute().GetNumericValue(AuraAttributeSet);
		AttributeInfoChangedDelegate.Broadcast(*Info);
	}
}

void UAttributesMenuController::BindCallbacksToDependencies()
{
	if (bCallbacksBound) return;
	
	bCallbacksBound = true;
}
