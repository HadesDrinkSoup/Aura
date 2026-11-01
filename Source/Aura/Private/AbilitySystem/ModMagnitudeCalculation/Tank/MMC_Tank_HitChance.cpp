// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Tank/MMC_Tank_HitChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Tank_HitChance::UMMC_Tank_HitChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Tank_HitChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHitChance + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 上限钳制：最高 100
	return FMath::Clamp(Result, 0.f, 100.f);
}
