// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_HitChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 命中几率：88 + 敏捷 × 0.18 + 等级 × 0.2
	// 命中最高，远程攻击的稳定性是核心优势。
	constexpr float BaseHitChance = 88.f;

	constexpr float AgilityCoefficient = 0.18f;
	constexpr float LevelCoefficient = 0.2f;
}

UMMC_Ranger_HitChance::UMMC_Ranger_HitChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Ranger_HitChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHitChance + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 上限钳制：最高 100
	return FMath::Clamp(Result, 0.f, 100.f);
}
