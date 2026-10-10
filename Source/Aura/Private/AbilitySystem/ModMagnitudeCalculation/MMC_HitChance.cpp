// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_HitChance.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_HitChance::UMMC_HitChance()
{
	AgilityDefinition.AttributeToCapture = UAuraAttributeSet::GetAgilityAttribute();
	AgilityDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AgilityDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(AgilityDefinition);
}

float UMMC_HitChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// BaseHitChance + Agility * AgilityCoefficient + CharacterLevel * LevelCoefficient
	static constexpr float BaseHitChance = 80.f;
	static constexpr float AgilityCoefficient = 0.1f;
	static constexpr float LevelCoefficient = 0.05f;

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

	return BaseHitChance + Agility * AgilityCoefficient + CharacterLevel * LevelCoefficient;
}
