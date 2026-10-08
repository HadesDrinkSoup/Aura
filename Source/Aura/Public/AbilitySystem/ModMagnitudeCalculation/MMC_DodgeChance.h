// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_DodgeChance.generated.h"

/**
 * 闪避率 = Agility * 0.5（百分比，范围 0-100）
 */
UCLASS()
class AURA_API UMMC_DodgeChance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_DodgeChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition AgilityDefinition;
};
