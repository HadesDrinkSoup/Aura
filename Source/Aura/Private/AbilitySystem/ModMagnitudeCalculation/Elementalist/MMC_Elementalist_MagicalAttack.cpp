// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_MagicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 元素法师 · 魔法攻击力：12 + 智力 × 3.4 + 等级 × 1.5
	// 核心输出：智力收益三职业最高。
	constexpr float BaseMagicalAttack = 12.f;

	constexpr float IntelligenceCoefficient = 3.4f;
	constexpr float LevelCoefficient = 1.5f;
}

UMMC_Elementalist_MagicalAttack::UMMC_Elementalist_MagicalAttack()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Elementalist_MagicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseMagicalAttack + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
