// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_PhysicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 物理攻击力：3 + 力量 × 1 + 敏捷 × 0.3 + 等级 × 0.6
	// 几乎不依赖武器，物理攻击只作兜底输出。
	constexpr float BasePhysicalAttack = 3.f;

	constexpr float StrengthCoefficient = 1.f;
	constexpr float AgilityCoefficient = 0.3f;
	constexpr float LevelCoefficient = 0.6f;
}

UMMC_Elementalist_PhysicalAttack::UMMC_Elementalist_PhysicalAttack()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetStrengthAttribute(),
		UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Elementalist_PhysicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BasePhysicalAttack + Strength * StrengthCoefficient + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
