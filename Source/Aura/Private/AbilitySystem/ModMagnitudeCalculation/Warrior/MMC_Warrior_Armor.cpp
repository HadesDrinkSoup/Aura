// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_Armor.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 护甲：5 + 活力 × 1.8 + 力量 × 1.3 + 等级 × 0.55
	// 护甲最高：活力 + 力量双加成，物理减伤核心。
	constexpr float BaseArmor = 5.f;

	constexpr float VigorCoefficient = 1.8f;
	constexpr float StrengthCoefficient = 1.3f;
	constexpr float LevelCoefficient = 0.55f;
}

UMMC_Warrior_Armor::UMMC_Warrior_Armor()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

float UMMC_Warrior_Armor::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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
