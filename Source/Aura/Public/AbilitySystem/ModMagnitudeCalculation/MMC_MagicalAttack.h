// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MagicalAttack.generated.h"

/**
 * 魔法攻击力 = BaseMagicalAttack + Intelligence * 4 + CharacterLevel * 2
 */
UCLASS()
class AURA_API UMMC_MagicalAttack : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_MagicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition IntelligenceDefinition;
};
