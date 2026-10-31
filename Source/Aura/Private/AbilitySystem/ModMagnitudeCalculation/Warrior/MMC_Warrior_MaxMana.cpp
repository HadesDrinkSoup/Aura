// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_MaxMana.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 最大法力值：25 + 智力 × 3.5 + 活力 × 1 + 等级 × 2.5
	// 法力池最小，只够支撑少量战技。
	constexpr float BaseMana = 25.f;

	constexpr float IntelligenceCoefficient = 3.5f;
	constexpr float VigorCoefficient = 1.f;
	constexpr float LevelCoefficient = 2.5f;
}

UMMC_Warrior_MaxMana::UMMC_Warrior_MaxMana()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetIntelligenceAttribute(),
		UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Warrior_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
