// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MagicResistance.generated.h"

/**
 * 战士 · 魔法抗性
 * 公式：1 + 活力 × 1.6 + 智力 × 0.15 + 等级 × 1.1（钳制：最低 0）
 * 魔抗次高：活力同时产出护甲与魔抗，前排扛物也扛法，仅次于坦克；满级约 150。
 */
UCLASS()
class AURA_API UMMC_Warrior_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_MagicResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 魔法抗性：1 + 活力 × 1.6 + 智力 × 0.15 + 等级 × 1.1
	// 魔抗次高：活力同时产出护甲与魔抗，前排扛物也扛法，仅次于坦克；满级约 150。
	static constexpr float BaseMagicResistance = 1.f;

	static constexpr float VigorCoefficient = 1.6f;
	static constexpr float IntelligenceCoefficient = 0.15f;
	static constexpr float LevelCoefficient = 1.1f;
};
