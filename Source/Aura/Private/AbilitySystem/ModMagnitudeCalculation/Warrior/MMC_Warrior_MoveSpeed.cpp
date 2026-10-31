// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_MoveSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 移动速度：390 + 敏捷 × 0.3 + 力量 × 0.05
	// 移速中等，力量提供极少额外移速。
	constexpr float BaseMoveSpeed = 390.f;

	constexpr float AgilityCoefficient = 0.3f;
	constexpr float StrengthCoefficient = 0.05f;
}

UMMC_Warrior_MoveSpeed::UMMC_Warrior_MoveSpeed()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Warrior_MoveSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);

	const float Result = BaseMoveSpeed + Agility * AgilityCoefficient + Strength * StrengthCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
