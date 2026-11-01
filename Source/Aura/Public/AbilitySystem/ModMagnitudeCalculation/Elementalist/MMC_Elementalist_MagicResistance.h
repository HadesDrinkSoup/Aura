// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MagicResistance.generated.h"

/**
 * 元素法师 · 魔法抗性
 * 公式：3 + 活力 × 0.45 + 智力 × 0.35 + 等级 × 0.4（钳制：最低 0）
 * 魔抗不再是强项：智力收益压到 0.35、活力 0.45，只比游侠高一点，被法术打到同样痛。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_MagicResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 魔法抗性：3 + 活力 × 0.45 + 智力 × 0.35 + 等级 × 0.4
	// 魔抗不再是强项：智力收益压到 0.35、活力 0.45，只比游侠高一点，被法术打到同样痛。
	static constexpr float BaseMagicResistance = 3.f;

	static constexpr float VigorCoefficient = 0.45f;
	static constexpr float IntelligenceCoefficient = 0.35f;
	static constexpr float LevelCoefficient = 0.4f;
};
