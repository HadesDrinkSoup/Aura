// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_PhysicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 物理攻击力：12 + 力量 × 3.2 + 敏捷 × 0.4 + 等级 × 1.5
	// 核心输出：力量收益三职业最高。
	constexpr float BasePhysicalAttack = 12.f;

	constexpr float StrengthCoefficient = 3.2f;
	constexpr float AgilityCoefficient = 0.4f;
	constexpr float LevelCoefficient = 1.5f;
}

UMMC_Warrior_PhysicalAttack::UMMC_Warrior_PhysicalAttack()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetStrengthAttribute(),
		UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Warrior_PhysicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
