// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraGameplayEffectContext.h"
#include "AuraAbilitySystemGlobals.generated.h"

UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
public:
	/**
	 * 每个 GAS 上下文都由这里分配，所以只要本类在配置里生效，全项目的 EffectContext 都是 FAuraGameplayEffectContext。
	 *
	 * 注意不能用 NewObject：FGameplayEffectContext 是 USTRUCT（F 前缀），不是 UObject，
	 */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override { return new FAuraGameplayEffectContext(); }
};
