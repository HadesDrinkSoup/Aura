// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_HealthRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 生命恢复：0.5 + 活力 × 0.08 + 等级 × 0.03
	// 基础回血最低，续航依赖法力与技能。
	constexpr float BaseHealthRegen = 0.5f;

	constexpr float VigorCoefficient = 0.08f;
	constexpr float LevelCoefficient = 0.03f;
}

UMMC_Elementalist_HealthRegen::UMMC_Elementalist_HealthRegen()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Elementalist_HealthRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHealthRegen + Vigor * VigorCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
