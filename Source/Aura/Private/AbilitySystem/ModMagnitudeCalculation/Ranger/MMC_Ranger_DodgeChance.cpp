// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_DodgeChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 闪避几率：6 + 敏捷 × 0.18
	// 闪避核心职业：敏捷收益最高。
	constexpr float BaseDodgeChance = 6.f;

	constexpr float AgilityCoefficient = 0.18f;
}

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
