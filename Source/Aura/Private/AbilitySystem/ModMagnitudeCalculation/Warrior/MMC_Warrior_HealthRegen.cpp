// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_HealthRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 生命恢复：1.5 + 活力 × 0.18 + 等级 × 0.06
	// 回血最高，配合高血量形成持续作战能力。
	constexpr float BaseHealthRegen = 1.5f;

	constexpr float VigorCoefficient = 0.18f;
	constexpr float LevelCoefficient = 0.06f;
}

UMMC_Warrior_HealthRegen::UMMC_Warrior_HealthRegen()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetVigorAttribute());
}

float UMMC_Warrior_HealthRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHealthRegen + Vigor * VigorCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
