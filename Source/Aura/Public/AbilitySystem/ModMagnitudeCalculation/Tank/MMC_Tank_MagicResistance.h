// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_MagicResistance.generated.h"

/**
 * 坦克 · 魔法抗性
 * 公式：4 + 活力 × 1.4 + 智力 × 0.3 + 等级 × 1.4（钳制：最低 0）
 * 魔抗最高：活力主导、智力补足，生命 / 护甲 / 魔抗三项全部第一，满级约 200。
 */
UCLASS()
class AURA_API UMMC_Tank_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_MagicResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 魔法抗性：4 + 活力 × 1.4 + 智力 × 0.3 + 等级 × 1.4
	// 魔抗最高：活力主导、智力补足，生命 / 护甲 / 魔抗三项全部第一；满级约 200。
	static constexpr float BaseMagicResistance = 4.f;

	static constexpr float VigorCoefficient = 1.4f;
	static constexpr float IntelligenceCoefficient = 0.3f;
	static constexpr float LevelCoefficient = 1.4f;
};
