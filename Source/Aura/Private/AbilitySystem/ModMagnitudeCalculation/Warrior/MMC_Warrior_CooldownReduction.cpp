// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_CooldownReduction.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 冷却缩减：0 + 智力 × 0.08 + 等级 × 0.03
	// 冷却缩减最低，技能循环明显偏慢。
	constexpr float BaseCooldownReduction = 0.f;

	constexpr float IntelligenceCoefficient = 0.08f;
	constexpr float LevelCoefficient = 0.03f;
}

UMMC_Warrior_CooldownReduction::UMMC_Warrior_CooldownReduction()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Warrior_CooldownReduction::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseCooldownReduction + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 上限钳制：最高 40
	return FMath::Clamp(Result, 0.f, 40.f);
}
