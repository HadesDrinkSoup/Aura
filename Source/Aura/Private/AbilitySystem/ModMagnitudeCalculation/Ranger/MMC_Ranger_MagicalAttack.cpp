// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_MagicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 游侠 · 魔法攻击力：4 + 智力 × 1.5 + 敏捷 × 0.4 + 等级 × 0.7
	// 魔法输出为辅，敏捷提供少量额外法强。
	constexpr float BaseMagicalAttack = 4.f;

	constexpr float IntelligenceCoefficient = 1.5f;
	constexpr float AgilityCoefficient = 0.4f;
	constexpr float LevelCoefficient = 0.7f;
}

UMMC_Ranger_MagicalAttack::UMMC_Ranger_MagicalAttack()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetIntelligenceAttribute(),
		UAuraAttributeSet::GetAgilityAttribute());
}

float UMMC_Ranger_MagicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseMagicalAttack + Intelligence * IntelligenceCoefficient + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
