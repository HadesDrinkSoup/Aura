// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CriticalChance.generated.h"

/**
 * 暴击率 = Agility * 0.5 + Intelligence * 0.25（百分比，范围 0-100）
 */
UCLASS()
class AURA_API UMMC_CriticalChance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_CriticalChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition AgilityDefinition;

	FGameplayEffectAttributeCaptureDefinition IntelligenceDefinition;
};
