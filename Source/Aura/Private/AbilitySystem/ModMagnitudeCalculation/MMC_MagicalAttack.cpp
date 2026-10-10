// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/MMC_MagicalAttack.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

UMMC_MagicalAttack::UMMC_MagicalAttack()
{
	IntelligenceDefinition.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDefinition.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceDefinition);
}

float UMMC_MagicalAttack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// BaseMagicalAttack + Intelligence * IntelligenceCoefficient + CharacterLevel * LevelCoefficient
	static constexpr float BaseMagicalAttack = 5.f;
	static constexpr float IntelligenceCoefficient = 2.5f;
	static constexpr float LevelCoefficient = 1.f;

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

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

	return BaseMagicalAttack + Intelligence * IntelligenceCoefficient + CharacterLevel * LevelCoefficient;
}
