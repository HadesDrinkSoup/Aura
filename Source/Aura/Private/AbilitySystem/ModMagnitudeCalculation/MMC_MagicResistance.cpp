// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_MagicResistance.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

// Vigor * VigorCoefficient + Intelligence * IntelligenceCoefficient + CharacterLevel * LevelCoefficient
static constexpr float BaseMagicResistance = 1.f;
static constexpr float VigorCoefficient = 0.8f;
static constexpr float IntelligenceCoefficient = 0.2f;
static constexpr float LevelCoefficient = 0.2f;

UMMC_MagicResistance::UMMC_MagicResistance()
{
	VigorDefinition.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDefinition.bSnapshot = false;

	IntelligenceDefinition.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDefinition);
	RelevantAttributesToCapture.Add(IntelligenceDefinition);
}

float UMMC_MagicResistance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDefinition, Spec, EvaluateParameters, Vigor);
	Vigor = FMath::Max(Vigor, 0.f);

	float Intelligence = 0.f;
	GetCapturedAttributeMagnitude(IntelligenceDefinition, Spec, EvaluateParameters, Intelligence);
	Intelligence = FMath::Max(Intelligence, 0.f);

	int32 CharacterLevel = 0;
	if (UObject* SourceObject = Spec.GetContext().GetSourceObject())
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(SourceObject))
		{
			CharacterLevel = CombatInterface->GetCharacterLevel();
		}
	}

	return BaseMagicResistance + Vigor * VigorCoefficient + Intelligence * IntelligenceCoefficient + CharacterLevel * LevelCoefficient;
}
