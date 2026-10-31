// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_HealthRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 生命恢复：0.8 + 活力 × 0.1 + 等级 × 0.04
	// 回血中等，续航依赖脱战与闪避。
	constexpr float BaseHealthRegen = 0.8f;

	constexpr float VigorCoefficient = 0.1f;
	constexpr float LevelCoefficient = 0.04f;
}

UMMC_Ranger_HealthRegen::UMMC_Ranger_HealthRegen()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Ranger_HealthRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHealthRegen + Vigor * VigorCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
