// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_MoveSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 移动速度：380 + 敏捷 × 0.25
	// 移速最慢，靠位移技能弥补机动性。
	constexpr float BaseMoveSpeed = 380.f;

	constexpr float AgilityCoefficient = 0.25f;
}

UMMC_Elementalist_MoveSpeed::UMMC_Elementalist_MoveSpeed()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Elementalist_MoveSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseMoveSpeed + Agility * AgilityCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
