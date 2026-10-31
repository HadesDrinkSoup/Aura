// Study Game Ability System _Hades


#include "UI/WidgetController/EnemyHealthBarWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

void UEnemyHealthBarWidgetController::BroadCastInitialValues()
{
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth(), -1.f);
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth(), -1.f);
}

void UEnemyHealthBarWidgetController::BindCallbacksToDependencies()
{
	if (bCallbacksBound) return;
	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute())
	.AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnHealthChanged.Broadcast(Data.NewValue, Data.OldValue);
	});
	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute())
	.AddLambda([this](const FOnAttributeChangeData& Data)
	{
		OnMaxHealthChanged.Broadcast(Data.NewValue, Data.OldValue);
	});
	bCallbacksBound = true;
}
