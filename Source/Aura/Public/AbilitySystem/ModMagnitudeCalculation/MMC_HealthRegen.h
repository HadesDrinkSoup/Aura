// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_HealthRegen.generated.h"

/**
 * 生命回复 = Vigor * 0.5 + CharacterLevel * 0.25（每秒）
 */
UCLASS()
class AURA_API UMMC_HealthRegen : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_HealthRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition VigorDefinition;
};
