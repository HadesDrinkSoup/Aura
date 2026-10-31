// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_MaxMana.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 最大法力值：120 + 智力 × 15 + 活力 × 2 + 等级 × 9
	// 法力池最大，智力收益最高，支撑长时间施法。
	constexpr float BaseMana = 120.f;

	constexpr float IntelligenceCoefficient = 15.f;
	constexpr float VigorCoefficient = 2.f;
	constexpr float LevelCoefficient = 9.f;
}

UMMC_Elementalist_MaxMana::UMMC_Elementalist_MaxMana()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetIntelligenceAttribute(),
		UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Elementalist_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseMana + Intelligence * IntelligenceCoefficient + Vigor * VigorCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
