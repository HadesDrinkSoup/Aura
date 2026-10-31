// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_MagicResistance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 魔法抗性：2 + 活力 × 0.5 + 智力 × 0.5 + 等级 × 0.25
	// 魔抗中等，活力与智力均衡提供。
	constexpr float BaseMagicResistance = 2.f;

	constexpr float VigorCoefficient = 0.5f;
	constexpr float IntelligenceCoefficient = 0.5f;
	constexpr float LevelCoefficient = 0.25f;
}

UMMC_Ranger_MagicResistance::UMMC_Ranger_MagicResistance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Ranger_MagicResistance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
