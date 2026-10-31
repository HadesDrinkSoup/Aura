// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_MagicResistance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 魔法抗性：1 + 活力 × 0.8 + 智力 × 0.15 + 等级 × 0.15
	// 魔抗偏低，需要装备弥补法术承伤。
	constexpr float BaseMagicResistance = 1.f;

	constexpr float VigorCoefficient = 0.8f;
	constexpr float IntelligenceCoefficient = 0.15f;
	constexpr float LevelCoefficient = 0.15f;
}

UMMC_Warrior_MagicResistance::UMMC_Warrior_MagicResistance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Warrior_MagicResistance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
