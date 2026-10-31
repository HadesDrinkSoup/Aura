// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_MagicResistance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 魔法抗性：3 + 活力 × 0.6 + 智力 × 1.2 + 等级 × 0.4
	// 智力兼顾魔抗，法系内战中抗性最好。
	constexpr float BaseMagicResistance = 3.f;

	constexpr float VigorCoefficient = 0.6f;
	constexpr float IntelligenceCoefficient = 1.2f;
	constexpr float LevelCoefficient = 0.4f;
}

UMMC_Elementalist_MagicResistance::UMMC_Elementalist_MagicResistance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Elementalist_MagicResistance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseMagicResistance + Vigor * VigorCoefficient + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
