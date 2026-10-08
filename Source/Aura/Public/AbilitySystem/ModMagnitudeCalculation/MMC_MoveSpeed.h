// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MoveSpeed.generated.h"

/**
 * 移动速度 = BaseMoveSpeed(300) + Agility * 2（单位/秒）
 */
UCLASS()
class AURA_API UMMC_MoveSpeed : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_MoveSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition AgilityDefinition;
};
