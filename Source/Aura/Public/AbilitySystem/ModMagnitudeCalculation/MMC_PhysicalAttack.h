// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_PhysicalAttack.generated.h"

/**
 * 物理攻击力 = BasePhysicalAttack + Strength * 4 + Agility * 1 + CharacterLevel * 2
 */
UCLASS()
class AURA_API UMMC_PhysicalAttack : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_PhysicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition StrengthDefinition;

	FGameplayEffectAttributeCaptureDefinition AgilityDefinition;
};
