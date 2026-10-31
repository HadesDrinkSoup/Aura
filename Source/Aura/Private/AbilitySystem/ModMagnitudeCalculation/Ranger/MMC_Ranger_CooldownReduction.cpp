// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_CooldownReduction.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 冷却缩减：1 + 敏捷 × 0.1 + 智力 × 0.05
	// 冷却缩减一般，主要来自敏捷。
	constexpr float BaseCooldownReduction = 1.f;

	constexpr float AgilityCoefficient = 0.1f;
	constexpr float IntelligenceCoefficient = 0.05f;
}

UMMC_Ranger_CooldownReduction::UMMC_Ranger_CooldownReduction()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Ranger_CooldownReduction::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCooldownReduction + Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient;

	// 上限钳制：最高 40
	return FMath::Clamp(Result, 0.f, 40.f);
}
