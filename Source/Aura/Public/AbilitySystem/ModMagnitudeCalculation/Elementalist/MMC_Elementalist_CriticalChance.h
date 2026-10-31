// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_CriticalChance.generated.h"

/**
 * 元素法师 · 暴击几率
 * 公式：5 + 智力 × 0.1 + 敏捷 × 0.05（钳制：0 ~ 75）
 * 暴击主要来自智力，敏捷只提供少量补充。
 */
UCLASS()
class AURA_API UMMC_Elementalist_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_CriticalChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 暴击几率：5 + 智力 × 0.1 + 敏捷 × 0.05
	// 暴击主要来自智力，敏捷只提供少量补充。
	static constexpr float BaseCriticalChance = 5.f;

	static constexpr float IntelligenceCoefficient = 0.1f;
	static constexpr float AgilityCoefficient = 0.05f;
};
