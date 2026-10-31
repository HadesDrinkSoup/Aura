// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_AttackSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 攻击速度：95 + 敏捷 × 0.15 + 等级 × 0.1
	// 基础攻速最低，普攻不是主要输出手段。
	constexpr float BaseAttackSpeed = 95.f;

	constexpr float AgilityCoefficient = 0.15f;
	constexpr float LevelCoefficient = 0.1f;
}

UMMC_Elementalist_AttackSpeed::UMMC_Elementalist_AttackSpeed()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Elementalist_AttackSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseAttackSpeed + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 区间钳制：50 ~ 250
	return FMath::Clamp(Result, 50.f, 250.f);
}
