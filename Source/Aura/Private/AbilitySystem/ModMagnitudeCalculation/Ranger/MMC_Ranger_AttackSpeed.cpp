// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_AttackSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 攻击速度：110 + 敏捷 × 0.45 + 等级 × 0.2
	// 攻速最高，高频普攻与技能衔接是核心玩法。
	constexpr float BaseAttackSpeed = 110.f;

	constexpr float AgilityCoefficient = 0.45f;
	constexpr float LevelCoefficient = 0.2f;
}

UMMC_Ranger_AttackSpeed::UMMC_Ranger_AttackSpeed()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Ranger_AttackSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseAttackSpeed + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 区间钳制：50 ~ 250
	return FMath::Clamp(Result, 50.f, 250.f);
}
