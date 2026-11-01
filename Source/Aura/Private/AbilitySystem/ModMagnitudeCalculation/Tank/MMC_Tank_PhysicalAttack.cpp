// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Tank/MMC_Tank_PhysicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Tank_PhysicalAttack::UMMC_Tank_PhysicalAttack()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetStrengthAttribute(),
		UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Tank_PhysicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);
	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BasePhysicalAttack + Strength * StrengthCoefficient + Vigor * VigorCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
