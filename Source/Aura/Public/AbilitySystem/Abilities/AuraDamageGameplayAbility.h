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
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Element")
	FGameplayTag DamageElementType;
	
	//命中时施加的伤害GE类
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category="ProjectileClass")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
