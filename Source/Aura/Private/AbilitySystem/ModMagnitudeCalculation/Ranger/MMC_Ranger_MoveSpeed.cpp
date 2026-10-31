// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_MoveSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 移动速度：420 + 敏捷 × 0.55
	// 移速最快，风筝与拉扯是核心优势。
	constexpr float BaseMoveSpeed = 420.f;

	constexpr float AgilityCoefficient = 0.55f;
}

UMMC_Ranger_MoveSpeed::UMMC_Ranger_MoveSpeed()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Ranger_MoveSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseMoveSpeed + Agility * AgilityCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
