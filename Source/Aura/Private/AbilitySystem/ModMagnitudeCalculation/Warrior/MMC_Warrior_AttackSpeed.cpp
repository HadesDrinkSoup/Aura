// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_AttackSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 攻击速度：100 + 敏捷 × 0.2 + 等级 × 0.1
	// 攻速中等，重视单次伤害而非攻击频率。
	constexpr float BaseAttackSpeed = 100.f;

	constexpr float AgilityCoefficient = 0.2f;
	constexpr float LevelCoefficient = 0.1f;
}

UMMC_Warrior_AttackSpeed::UMMC_Warrior_AttackSpeed()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Warrior_AttackSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseAttackSpeed + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 区间钳制：50 ~ 250
	return FMath::Clamp(Result, 50.f, 250.f);
}
