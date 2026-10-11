// Study Game Ability System _Hades

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGamepalyTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

void UAuraAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (const TSubclassOf AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = CastChecked<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->AbilityInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

// -----------------------------------------------------------------------------
// 能力输入处理
// 三个函数由 AAuraPlayerController 的输入回调转发而来，分别对应
// Enhanced Input 的 Started / Triggered / Completed。
// 这里按 GameplayTag 匹配能力（Tag 在 AddCharacterAbilities 中写入 DynamicAbilityTags），
// 再调用基类 AbilitySpecInputPressed / AbilitySpecInputReleased 把输入事件派发给能力实例。
// -----------------------------------------------------------------------------

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// 遍历所有已授予且可激活的能力，找到 DynamicAbilityTags 精确匹配 InputTag 的 Spec
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 标记「按下」并派发给能力实例（基类已处理 InstancedPerExecution 的多实例遍历）
			AbilitySpecInputPressed(AbilitySpec);
			// 能力处于激活中时广播 InputPressed 复制事件，供 WaitInputPress 等 GameplayTask 监听
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, FPredictionKey());
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 松开 → 基类将 InputPressed 置 false，并派发 InputReleased 给能力实例
			// （蓄力类技能通常在松开的瞬间触发真正施放）
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// 持续按住 → 同样标记按下状态
			AbilitySpecInputPressed(AbilitySpec);
			// Triggered 每帧触发，首次触发时能力通常尚未激活，故在此激活
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle) const
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTagsDelegate.Broadcast(TagContainer);
}
