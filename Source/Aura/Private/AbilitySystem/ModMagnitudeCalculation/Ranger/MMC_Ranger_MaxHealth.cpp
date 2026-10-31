// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 最大生命值：90 + 活力 × 9 + 力量 × 4 + 等级 × 9
	// 血量中等：靠闪避与走位生存，而不是硬抗伤害。
	constexpr float BaseHealth = 90.f;

	constexpr float VigorCoefficient = 9.f;
	constexpr float StrengthCoefficient = 4.f;
	constexpr float LevelCoefficient = 9.f;
}

UMMC_Ranger_MaxHealth::UMMC_Ranger_MaxHealth()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Ranger_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
