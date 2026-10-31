// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_HitChance.generated.h"

/**
 * 元素法师 · 命中几率
 * 公式：80 + 敏捷 × 0.06 + 智力 × 0.05 + 等级 × 0.1（钳制：0 ~ 100）
 * 命中偏低，需要装备或天赋补足。
 */
UCLASS()
class AURA_API UMMC_Elementalist_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_HitChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 命中几率：80 + 敏捷 × 0.06 + 智力 × 0.05 + 等级 × 0.1
	// 命中偏低，需要装备或天赋补足。
	static constexpr float BaseHitChance = 80.f;

	static constexpr float AgilityCoefficient = 0.06f;
	static constexpr float IntelligenceCoefficient = 0.05f;
	static constexpr float LevelCoefficient = 0.1f;
};
