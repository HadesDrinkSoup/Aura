// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_AttackSpeed.generated.h"

/**
 * 攻击速度 = BaseAttackSpeed(100) + Agility * 1.5 + CharacterLevel * 0.5（基础100%）
 */
UCLASS()
class AURA_API UMMC_AttackSpeed : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_AttackSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition AgilityDefinition;
};
