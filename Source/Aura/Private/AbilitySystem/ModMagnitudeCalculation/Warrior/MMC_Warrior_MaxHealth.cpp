// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 最大生命值：140 + 活力 × 15 + 力量 × 9 + 等级 × 15
	// 血量最高：活力与力量双收益，是前排承伤的核心。
	constexpr float BaseHealth = 140.f;

	constexpr float VigorCoefficient = 15.f;
	constexpr float StrengthCoefficient = 9.f;
	constexpr float LevelCoefficient = 15.f;
}

UMMC_Warrior_MaxHealth::UMMC_Warrior_MaxHealth()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Warrior_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHealth + Vigor * VigorCoefficient + Strength * StrengthCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 1
	return FMath::Max(Result, 1.f);
}
