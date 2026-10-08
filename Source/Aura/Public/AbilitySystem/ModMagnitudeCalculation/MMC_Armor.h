// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_Armor.generated.h"

/**
 * 护甲 = Vigor * 3 + Strength * 1 + CharacterLevel * 1
 */
UCLASS()
class AURA_API UMMC_Armor : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_Armor();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition VigorDefinition;

	FGameplayEffectAttributeCaptureDefinition StrengthDefinition;
};
