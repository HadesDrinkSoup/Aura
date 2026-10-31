// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_DodgeChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Ranger_DodgeChance::UMMC_Ranger_DodgeChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Ranger_DodgeChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseDodgeChance + Agility * AgilityCoefficient;

	// 上限钳制：最高 60
	return FMath::Clamp(Result, 0.f, 60.f);
}
