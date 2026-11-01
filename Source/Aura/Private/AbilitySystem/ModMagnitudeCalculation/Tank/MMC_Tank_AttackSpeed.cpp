// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Tank/MMC_Tank_AttackSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Tank_AttackSpeed::UMMC_Tank_AttackSpeed()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Tank_AttackSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseAttackSpeed + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 区间钳制：50 ~ 250
	return FMath::Clamp(Result, 50.f, 250.f);
}
