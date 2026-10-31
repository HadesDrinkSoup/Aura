// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 最大生命值：65 + 活力 × 7 + 力量 × 2 + 等级 × 7
	// 体魄最弱：血量主要来自等级，活力收益明显低于战士。
	constexpr float BaseHealth = 65.f;

	constexpr float VigorCoefficient = 7.f;
	constexpr float StrengthCoefficient = 2.f;
	constexpr float LevelCoefficient = 7.f;
}

UMMC_Elementalist_MaxHealth::UMMC_Elementalist_MaxHealth()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Elementalist_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHealth + Vigor * VigorCoefficient + Strength * StrengthCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 1
	return FMath::Max(Result, 1.f);
}
