// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_PhysicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 物理攻击力：8 + 力量 × 1.3 + 敏捷 × 2.4 + 等级 × 1.2
	// 敏捷是主要加成，力量提供基础底子。
	constexpr float BasePhysicalAttack = 8.f;

	constexpr float StrengthCoefficient = 1.3f;
	constexpr float AgilityCoefficient = 2.4f;
	constexpr float LevelCoefficient = 1.2f;
}

UMMC_Ranger_PhysicalAttack::UMMC_Ranger_PhysicalAttack()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetStrengthAttribute(),
		UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Ranger_PhysicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
