// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_CriticalChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Ranger_CriticalChance::UMMC_Ranger_CriticalChance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Ranger_CriticalChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCriticalChance + Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient;

	// 上限钳制：最高 75
	return FMath::Clamp(Result, 0.f, 75.f);
}
