// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_CriticalChance.generated.h"

/**
 * 战士 · 暴击几率
 * 公式：5 + 敏捷 × 0.08 + 力量 × 0.04（钳制：0 ~ 75）
 * 暴击来自敏捷与力量的稳定加成，成长平缓。
 */
UCLASS()
class AURA_API UMMC_Warrior_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_CriticalChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
