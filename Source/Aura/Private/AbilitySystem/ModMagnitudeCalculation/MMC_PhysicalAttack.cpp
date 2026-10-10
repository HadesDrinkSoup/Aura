// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_PhysicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_PhysicalAttack::UMMC_PhysicalAttack()
{
	StrengthDefinition.AttributeToCapture = UAuraAttributeSet::GetStrengthAttribute();
	StrengthDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthDefinition.bSnapshot = false;

	AgilityDefinition.AttributeToCapture = UAuraAttributeSet::GetAgilityAttribute();
	AgilityDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AgilityDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(StrengthDefinition);
	RelevantAttributesToCapture.Add(AgilityDefinition);
}

float UMMC_PhysicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// BasePhysicalAttack + Strength * StrengthCoefficient + Agility * AgilityCoefficient + CharacterLevel * LevelCoefficient
	static constexpr float BasePhysicalAttack = 5.f;
	static constexpr float StrengthCoefficient = 2.5f;
	static constexpr float AgilityCoefficient = 0.5f;
	static constexpr float LevelCoefficient = 1.f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Strength = 0.f;
	GetCapturedAttributeMagnitude(StrengthDefinition, Spec, EvaluateParameters, Strength);
	Strength = FMath::Max(Strength, 0.f);

	float Agility = 0.f;
	GetCapturedAttributeMagnitude(AgilityDefinition, Spec, EvaluateParameters, Agility);
	Agility = FMath::Max(Agility, 0.f);

	int32 CharacterLevel = 0;
	if (UObject* SourceObject = Spec.GetContext().GetSourceObject())
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(SourceObject))
		{
			CharacterLevel = CombatInterface->GetCharacterLevel();
		}
	}

	return BasePhysicalAttack + Strength * StrengthCoefficient + Agility * AgilityCoefficient + CharacterLevel * LevelCoefficient;
}
