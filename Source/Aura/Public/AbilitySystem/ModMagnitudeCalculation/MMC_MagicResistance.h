// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MagicResistance.generated.h"

/**
 * 魔法抗性 = Vigor * 2 + Intelligence * 2 + CharacterLevel * 1
 */
UCLASS()
class AURA_API UMMC_MagicResistance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_MagicResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition VigorDefinition;

	FGameplayEffectAttributeCaptureDefinition IntelligenceDefinition;
};
