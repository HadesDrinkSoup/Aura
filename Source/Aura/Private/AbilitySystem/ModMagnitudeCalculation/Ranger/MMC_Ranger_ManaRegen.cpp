// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_ManaRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 法力恢复：1 + 智力 × 0.12 + 敏捷 × 0.04 + 等级 × 0.04
	// 回蓝中等，敏捷提供少量额外回复。
	constexpr float BaseManaRegen = 1.f;

	constexpr float IntelligenceCoefficient = 0.12f;
	constexpr float AgilityCoefficient = 0.04f;
	constexpr float LevelCoefficient = 0.04f;
}

UMMC_Ranger_ManaRegen::UMMC_Ranger_ManaRegen()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetIntelligenceAttribute(),
		UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Ranger_ManaRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseManaRegen + Intelligence * IntelligenceCoefficient + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
