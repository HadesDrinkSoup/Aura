// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_Armor.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_Armor::UMMC_Armor()
{
	VigorDefinition.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDefinition.bSnapshot = false;

	StrengthDefinition.AttributeToCapture = UAuraAttributeSet::GetStrengthAttribute();
	StrengthDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDefinition);
	RelevantAttributesToCapture.Add(StrengthDefinition);
}

float UMMC_Armor::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Vigor * VigorCoefficient + Strength * StrengthCoefficient + CharacterLevel * LevelCoefficient
	static constexpr float BaseArmor = 1.f;
	static constexpr float VigorCoefficient = 1.f;
	static constexpr float StrengthCoefficient = 0.3f;
	static constexpr float LevelCoefficient = 0.2f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDefinition, Spec, EvaluateParameters, Vigor);
	Vigor = FMath::Max(Vigor, 0.f);

	float Strength = 0.f;
	GetCapturedAttributeMagnitude(StrengthDefinition, Spec, EvaluateParameters, Strength);
	Strength = FMath::Max(Strength, 0.f);

	int32 CharacterLevel = 0;
	if (UObject* SourceObject = Spec.GetContext().GetSourceObject())
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(SourceObject))
		{
			CharacterLevel = CombatInterface->GetCharacterLevel();
		}
	}

	return BaseArmor + Vigor * VigorCoefficient + Strength * StrengthCoefficient + CharacterLevel * LevelCoefficient;
}
