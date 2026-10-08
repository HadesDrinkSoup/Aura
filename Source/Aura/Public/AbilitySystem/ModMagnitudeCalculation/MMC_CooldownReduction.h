// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_CooldownReduction.generated.h"

/**
 * 冷却缩减 = Intelligence * 0.5（百分比，范围 0-100）
 */
UCLASS()
class AURA_API UMMC_CooldownReduction : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_CooldownReduction();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition IntelligenceDefinition;
};
