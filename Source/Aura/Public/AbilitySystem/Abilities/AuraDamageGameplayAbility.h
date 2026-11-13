// Study Game Ability System _Hadess

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraDamageGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Ability|Damage", meta = (DisplayName = "伤害元素类型"))
	FGameplayTag DamageElementType;
	
	//命中时施加的伤害GE类
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "Aura|Ability|Damage", meta = (DisplayName = "伤害效果类"))
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
