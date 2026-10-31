// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_MagicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 魔法攻击力：2 + 智力 × 0.8 + 等级 × 0.3
	// 几乎无魔法输出，智力收益极低。
	constexpr float BaseMagicalAttack = 2.f;

	constexpr float IntelligenceCoefficient = 0.8f;
	constexpr float LevelCoefficient = 0.3f;
}

UMMC_Warrior_MagicalAttack::UMMC_Warrior_MagicalAttack()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Warrior_MagicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseMagicalAttack + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
