// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MagicResistance.generated.h"

/**
 * 战士 · 魔法抗性
 * 公式：1 + 活力 × 0.8 + 智力 × 0.15 + 等级 × 0.15（钳制：最低 0）
 * 魔抗偏低，需要装备弥补法术承伤。
 */
UCLASS()
class AURA_API UMMC_Warrior_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_MagicResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
