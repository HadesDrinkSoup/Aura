// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MagicResistance.generated.h"

/**
 * 元素法师 · 魔法抗性
 * 公式：3 + 活力 × 0.6 + 智力 × 1.2 + 等级 × 0.4（钳制：最低 0）
 * 智力兼顾魔抗，法系内战中抗性最好。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_MagicResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 魔法抗性：3 + 活力 × 0.6 + 智力 × 1.2 + 等级 × 0.4
	// 智力兼顾魔抗，法系内战中抗性最好。
	static constexpr float BaseMagicResistance = 3.f;

	static constexpr float VigorCoefficient = 0.6f;
	static constexpr float IntelligenceCoefficient = 1.2f;
	static constexpr float LevelCoefficient = 0.4f;
};
