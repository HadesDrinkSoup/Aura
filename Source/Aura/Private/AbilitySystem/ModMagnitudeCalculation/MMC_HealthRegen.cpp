// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_HealthRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_HealthRegen::UMMC_HealthRegen()
{
	VigorDefinition.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDefinition);
}

float UMMC_HealthRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Vigor * VigorCoefficient + CharacterLevel * LevelCoefficient
	static constexpr float BaseHealthRegen = 1.f;
	static constexpr float VigorCoefficient = 0.12f;
	static constexpr float LevelCoefficient = 0.05f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDefinition, Spec, EvaluateParameters, Vigor);
	Vigor = FMath::Max(Vigor, 0.f);

	int32 CharacterLevel = 0;
	if (UObject* SourceObject = Spec.GetContext().GetSourceObject())
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(SourceObject))
		{
			CharacterLevel = CombatInterface->GetCharacterLevel();
		}
	}

	return BaseHealthRegen + Vigor * VigorCoefficient + CharacterLevel * LevelCoefficient;
}
