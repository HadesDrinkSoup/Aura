// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Warrior/MMC_Warrior_ManaRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{
	// 战士 · 法力恢复：0.5 + 智力 × 0.05 + 等级 × 0.02
	// 回蓝最低，法力属于稀缺资源。
	constexpr float BaseManaRegen = 0.5f;

	constexpr float IntelligenceCoefficient = 0.05f;
	constexpr float LevelCoefficient = 0.02f;
}

UMMC_Warrior_ManaRegen::UMMC_Warrior_ManaRegen()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Warrior_ManaRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseManaRegen + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0
	return FMath::Max(Result, 0.f);
}
