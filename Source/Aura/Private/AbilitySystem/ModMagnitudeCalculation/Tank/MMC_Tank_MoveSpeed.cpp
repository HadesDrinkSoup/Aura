// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Tank/MMC_Tank_MoveSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Tank_MoveSpeed::UMMC_Tank_MoveSpeed()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Tank_MoveSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);

	const float Result = BaseMoveSpeed + Agility * AgilityCoefficient + Vigor * VigorCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
