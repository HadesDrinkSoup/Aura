// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_AttackSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_AttackSpeed::UMMC_AttackSpeed()
{
	AgilityDefinition.AttributeToCapture = UAuraAttributeSet::GetAgilityAttribute();
	AgilityDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AgilityDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(AgilityDefinition);
}

float UMMC_AttackSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// BaseAttackSpeed + Agility * AgilityCoefficient + CharacterLevel * LevelCoefficient
	static constexpr float BaseAttackSpeed = 100.f;
	static constexpr float AgilityCoefficient = 0.3f;
	static constexpr float LevelCoefficient = 0.1f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

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

	return BaseAttackSpeed + Agility * AgilityCoefficient + CharacterLevel * LevelCoefficient;
}
