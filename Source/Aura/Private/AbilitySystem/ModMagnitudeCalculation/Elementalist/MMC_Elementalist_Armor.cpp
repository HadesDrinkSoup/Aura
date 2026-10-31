// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_Armor.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 护甲：0 + 活力 × 0.8 + 力量 × 0.2 + 等级 × 0.15
	// 基础护甲为 0，只能获得极低的物理减免。
	constexpr float BaseArmor = 0.f;

	constexpr float VigorCoefficient = 0.8f;
	constexpr float StrengthCoefficient = 0.2f;
	constexpr float LevelCoefficient = 0.15f;
}

UMMC_Elementalist_Armor::UMMC_Elementalist_Armor()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Elementalist_Armor::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
