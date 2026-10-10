// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_DodgeChance.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_DodgeChance::UMMC_DodgeChance()
{
	AgilityDefinition.AttributeToCapture = UAuraAttributeSet::GetAgilityAttribute();
	AgilityDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AgilityDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(AgilityDefinition);
}

float UMMC_DodgeChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Agility * AgilityCoefficient
	static constexpr float AgilityCoefficient = 0.1f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Agility = 0.f;
	GetCapturedAttributeMagnitude(AgilityDefinition, Spec, EvaluateParameters, Agility);
	Agility = FMath::Max(Agility, 0.f);

	return Agility * AgilityCoefficient;
}
