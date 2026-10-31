// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_MaxMana.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 最大法力值：55 + 智力 × 6.5 + 活力 × 1.5 + 等级 × 4.5
	// 法力中等，足以支撑标记与位移类技能。
	constexpr float BaseMana = 55.f;

	constexpr float IntelligenceCoefficient = 6.5f;
	constexpr float VigorCoefficient = 1.5f;
	constexpr float LevelCoefficient = 4.5f;
}

UMMC_Ranger_MaxMana::UMMC_Ranger_MaxMana()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetIntelligenceAttribute(),
		UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Ranger_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
