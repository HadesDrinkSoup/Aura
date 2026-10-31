// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_DodgeChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 闪避几率：3 + 敏捷 × 0.07
	// 闪避成长最低，靠护甲而不是闪避生存。
	constexpr float BaseDodgeChance = 3.f;

	constexpr float AgilityCoefficient = 0.07f;
}

UMMC_Warrior_DodgeChance::UMMC_Warrior_DodgeChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Warrior_DodgeChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseDodgeChance + Agility * AgilityCoefficient;

	// 上限钳制：最高 60
	return FMath::Clamp(Result, 0.f, 60.f);
}
