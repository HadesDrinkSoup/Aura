// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_ManaRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 法力恢复：3 + 智力 × 0.3 + 等级 × 0.1
	// 回蓝核心职业，支撑高频技能循环。
	constexpr float BaseManaRegen = 3.f;

	constexpr float IntelligenceCoefficient = 0.3f;
	constexpr float LevelCoefficient = 0.1f;
}

UMMC_Elementalist_ManaRegen::UMMC_Elementalist_ManaRegen()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Elementalist_ManaRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseManaRegen + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
