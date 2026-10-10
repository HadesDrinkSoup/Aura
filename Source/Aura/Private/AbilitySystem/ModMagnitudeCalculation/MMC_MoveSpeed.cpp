// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_MoveSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_MoveSpeed::UMMC_MoveSpeed()
{
	AgilityDefinition.AttributeToCapture = UAuraAttributeSet::GetAgilityAttribute();
	AgilityDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AgilityDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(AgilityDefinition);
}

float UMMC_MoveSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// BaseMoveSpeed + Agility * AgilityCoefficient
	static constexpr float BaseMoveSpeed = 400.f;
	static constexpr float AgilityCoefficient = 0.5f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Agility = 0.f;
	GetCapturedAttributeMagnitude(AgilityDefinition, Spec, EvaluateParameters, Agility);
	Agility = FMath::Max(Agility, 0.f);

	return BaseMoveSpeed + Agility * AgilityCoefficient;
}
