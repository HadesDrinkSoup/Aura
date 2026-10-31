// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_Armor.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 护甲：1 + 活力 × 1 + 力量 × 0.35 + 等级 × 0.25
	// 护甲偏低，被近身时风险很高。
	constexpr float BaseArmor = 1.f;

	constexpr float VigorCoefficient = 1.f;
	constexpr float StrengthCoefficient = 0.35f;
	constexpr float LevelCoefficient = 0.25f;
}

UMMC_Ranger_Armor::UMMC_Ranger_Armor()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Ranger_Armor::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseArmor + Vigor * VigorCoefficient + Strength * StrengthCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
