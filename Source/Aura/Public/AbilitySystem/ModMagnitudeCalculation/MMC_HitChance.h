// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_HitChance.generated.h"

/**
 * 命中率 = BaseHitChance(75) + Agility * 1 + CharacterLevel * 0.5（百分比）
 */
UCLASS()
class AURA_API UMMC_HitChance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_HitChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition AgilityDefinition;
};
