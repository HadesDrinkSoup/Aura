// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_CriticalChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 暴击几率：5 + 敏捷 × 0.08 + 力量 × 0.04
	// 暴击来自敏捷与力量的稳定加成，成长平缓。
	constexpr float BaseCriticalChance = 5.f;

	constexpr float AgilityCoefficient = 0.08f;
	constexpr float StrengthCoefficient = 0.04f;
}

UMMC_Warrior_CriticalChance::UMMC_Warrior_CriticalChance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Warrior_CriticalChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCriticalChance + Agility * AgilityCoefficient + Strength * StrengthCoefficient;

	// 上限钳制：最高 75
	return FMath::Clamp(Result, 0.f, 75.f);
}
