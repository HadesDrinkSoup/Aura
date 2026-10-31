// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Elementalist/MMC_Elementalist_CooldownReduction.h"
#include "AbilitySystem/AuraAttributeSet.h"

UMMC_Elementalist_CooldownReduction::UMMC_Elementalist_CooldownReduction()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetIntelligenceAttribute());
}

float UMMC_Elementalist_CooldownReduction::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseCooldownReduction + Intelligence * IntelligenceCoefficient + Level * LevelCoefficient;

	// 上限钳制：最高 40
	return FMath::Clamp(Result, 0.f, 40.f);
}
