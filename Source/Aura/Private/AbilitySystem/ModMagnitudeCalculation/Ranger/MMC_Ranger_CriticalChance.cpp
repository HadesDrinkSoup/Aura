// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_CriticalChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 暴击几率：8 + 敏捷 × 0.2 + 智力 × 0.03
	// 暴击核心职业：敏捷收益三职业最高。
	constexpr float BaseCriticalChance = 8.f;

	constexpr float AgilityCoefficient = 0.2f;
	constexpr float IntelligenceCoefficient = 0.03f;
}

UMMC_Ranger_CriticalChance::UMMC_Ranger_CriticalChance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Ranger_CriticalChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCriticalChance + Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient;

	// 上限钳制：最高 75
	return FMath::Clamp(Result, 0.f, 75.f);
}
