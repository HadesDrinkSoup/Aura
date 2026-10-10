// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_CriticalChance.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_CriticalChance::UMMC_CriticalChance()
{
	AgilityDefinition.AttributeToCapture = UAuraAttributeSet::GetAgilityAttribute();
	AgilityDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AgilityDefinition.bSnapshot = false;

	IntelligenceDefinition.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(AgilityDefinition);
	RelevantAttributesToCapture.Add(IntelligenceDefinition);
}

float UMMC_CriticalChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient
	static constexpr float AgilityCoefficient = 0.15f;
	static constexpr float IntelligenceCoefficient = 0.1f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Agility = 0.f;
	GetCapturedAttributeMagnitude(AgilityDefinition, Spec, EvaluateParameters, Agility);
	Agility = FMath::Max(Agility, 0.f);

	float Intelligence = 0.f;
	GetCapturedAttributeMagnitude(IntelligenceDefinition, Spec, EvaluateParameters, Intelligence);
	Intelligence = FMath::Max(Intelligence, 0.f);

	return Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient;
}
