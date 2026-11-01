// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Tank/MMC_Tank_CriticalChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Tank_CriticalChance::UMMC_Tank_CriticalChance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetStrengthAttribute(),
		UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Tank_CriticalChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCriticalChance + Strength * StrengthCoefficient + Agility * AgilityCoefficient;

	// 上限钳制：最高 75
	return FMath::Clamp(Result, 0.f, 75.f);
}
