// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_ManaRegen.generated.h"

/**
 * 法力回复 = Intelligence * 0.5 + CharacterLevel * 0.25（每秒）
 */
UCLASS()
class AURA_API UMMC_ManaRegen : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_ManaRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition IntelligenceDefinition;
};
