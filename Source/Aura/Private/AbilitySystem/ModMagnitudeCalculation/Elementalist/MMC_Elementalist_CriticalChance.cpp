// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_CriticalChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 暴击几率：5 + 智力 × 0.1 + 敏捷 × 0.05
	// 暴击主要来自智力，敏捷只提供少量补充。
	constexpr float BaseCriticalChance = 5.f;

	constexpr float IntelligenceCoefficient = 0.1f;
	constexpr float AgilityCoefficient = 0.05f;
}

UMMC_Elementalist_CriticalChance::UMMC_Elementalist_CriticalChance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetIntelligenceAttribute(),
		UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Elementalist_CriticalChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCriticalChance + Intelligence * IntelligenceCoefficient + Agility * AgilityCoefficient;

	// 上限钳制：最高 75
	return FMath::Clamp(Result, 0.f, 75.f);
}
