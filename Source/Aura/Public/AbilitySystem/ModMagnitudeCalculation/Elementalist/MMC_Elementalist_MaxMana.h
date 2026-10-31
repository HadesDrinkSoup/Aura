// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MaxMana.generated.h"

/**
 * 元素法师 · 最大法力值
 * 公式：120 + 智力 × 15 + 活力 × 2 + 等级 × 9（钳制：最低 0）
 * 法力池最大，智力收益最高，支撑长时间施法。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MaxMana : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_MaxMana();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 最大法力值：120 + 智力 × 15 + 活力 × 2 + 等级 × 9
	// 法力池最大，智力收益最高，支撑长时间施法。
	static constexpr float BaseMana = 120.f;

	static constexpr float IntelligenceCoefficient = 15.f;
	static constexpr float VigorCoefficient = 2.f;
	static constexpr float LevelCoefficient = 9.f;
};
