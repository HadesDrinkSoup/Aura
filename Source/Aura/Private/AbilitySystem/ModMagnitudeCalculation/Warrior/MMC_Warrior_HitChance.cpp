// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_HitChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 命中几率：82 + 敏捷 × 0.08 + 等级 × 0.15
	// 命中稳定偏高，保证近战技能不落空。
	constexpr float BaseHitChance = 82.f;

	constexpr float AgilityCoefficient = 0.08f;
	constexpr float LevelCoefficient = 0.15f;
}

UMMC_Warrior_HitChance::UMMC_Warrior_HitChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Warrior_HitChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHitChance + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 上限钳制：最高 100
	return FMath::Clamp(Result, 0.f, 100.f);
}
