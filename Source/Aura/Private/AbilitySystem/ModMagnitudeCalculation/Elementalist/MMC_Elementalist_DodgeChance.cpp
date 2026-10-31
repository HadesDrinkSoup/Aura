// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_DodgeChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 闪避几率：2 + 敏捷 × 0.08
	// 闪避成长最差，不靠敏捷生存。
	constexpr float BaseDodgeChance = 2.f;

	constexpr float AgilityCoefficient = 0.08f;
}

UMMC_Elementalist_DodgeChance::UMMC_Elementalist_DodgeChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Elementalist_DodgeChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseDodgeChance + Agility * AgilityCoefficient;

	// 上限钳制：最高 60
	return FMath::Clamp(Result, 0.f, 60.f);
}
