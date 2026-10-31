// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_HitChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 命中几率：80 + 敏捷 × 0.06 + 智力 × 0.05 + 等级 × 0.1
	// 命中偏低，需要装备或天赋补足。
	constexpr float BaseHitChance = 80.f;

	constexpr float AgilityCoefficient = 0.06f;
	constexpr float IntelligenceCoefficient = 0.05f;
	constexpr float LevelCoefficient = 0.1f;
}

UMMC_Elementalist_HitChance::UMMC_Elementalist_HitChance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Elementalist_HitChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHitChance + Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 上限钳制：最高 100
	return FMath::Clamp(Result, 0.f, 100.f);
}
