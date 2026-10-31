// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_MagicResistance.generated.h"

/**
 * 游侠 · 魔法抗性
 * 公式：2 + 活力 × 0.5 + 智力 × 0.5 + 等级 × 0.25（钳制：最低 0）
 * 魔抗中等，活力与智力均衡提供。
 */
UCLASS()
class AURA_API UMMC_Ranger_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_MagicResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
