// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_CriticalChance.generated.h"

/**
 * 元素法师 · 暴击几率
 * 公式：4 + 智力 × 0.05 + 敏捷 × 0.04（钳制：0 ~ 75）
 * 暴击偏低：法系不靠爆发，智力只做温和加成，排在战士之后、坦克之前。
 */
UCLASS()
class AURA_API UMMC_Elementalist_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_CriticalChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 暴击几率：4 + 智力 × 0.05 + 敏捷 × 0.04
	// 暴击偏低：法系不靠爆发，智力只做温和加成，排在战士之后、坦克之前。
	static constexpr float BaseCriticalChance = 4.f;

	static constexpr float IntelligenceCoefficient = 0.05f;
	static constexpr float AgilityCoefficient = 0.04f;
};
