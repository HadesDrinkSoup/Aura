// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_MaxMana.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
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

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//BaseMana + Vigor * VigorCoefficient + Intelligence * IntelligenceCoefficient + CharacterLevel * LevelCoefficient;
	static constexpr float BaseMana = 45.f;
	static constexpr float VigorCoefficient = 2.f;
	static constexpr float IntelligenceCoefficient = 8.f;
	static constexpr float LevelCoefficient = 5.f;

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
	return BaseMana + Vigor * VigorCoefficient + Intelligence * IntelligenceCoefficient + CharacterLevel * LevelCoefficient;
}
