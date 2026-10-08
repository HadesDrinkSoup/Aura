// Study Game Ability System _Hades

#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

void UOverlayWidgetController::BroadCastInitialValues()
{
	// 广播当前的初始值
	// 这对于正确显示 UI 初始状态至关重要：
	//   - 如果不广播初始值，Widget 会显示默认的 0/0，直到第一次属性变化
	//   - 广播后，立即显示正确的当前数值
	OnHealthChangedDelegate.Broadcast(AuraAttributeSet->GetHealth(), -1.f);
	OnMaxHealthChangedDelegate.Broadcast(AuraAttributeSet->GetMaxHealth(), -1.f);
	OnManaChangedDelegate.Broadcast(AuraAttributeSet->GetMana(), -1.f);
	OnMaxManaChangedDelegate.Broadcast(AuraAttributeSet->GetMaxMana(), -1.f);
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	if (bCallbacksBound) return;

	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnHealthChangedDelegate.Broadcast(Data.NewValue, Data.OldValue);
		});
	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChangedDelegate.Broadcast(Data.NewValue, Data.OldValue);
		});
	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnManaChangedDelegate.Broadcast(Data.NewValue, Data.OldValue);
		});
	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChangedDelegate.Broadcast(Data.NewValue, Data.OldValue);
		});
	AuraAbilitySystemComponent->EffectAssetTagsDelegate.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			const FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
			for (const FGameplayTag& Tag : AssetTags)
			{
				if (Tag.MatchesTag(MessageTag))
				{
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row);
				}
			}
		});

	bCallbacksBound = true;
}
