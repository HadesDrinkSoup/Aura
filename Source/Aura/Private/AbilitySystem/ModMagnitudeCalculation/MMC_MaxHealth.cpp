// Study Game Ability System _Hades
// 中文注释已添加：用于说明该计算器的目的与注意点

#include "AbilitySystem/ModMagnitudeCalculation/MMC_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"

// 计算最大生命值的自定义 Magnitude Calculation
UMMC_MaxHealth::UMMC_MaxHealth()
{
	// 捕获目标的 Vigor 属性，用于后续计算最大生命值
	VigorDefinition.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	// 从目标（Target）捕获属性值；如需从施法者捕获请改为 Source
	VigorDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	// 是否对捕获值做快照（true：使用应用时的快照值，false：动态读取）
	VigorDefinition.bSnapshot = false;
	
	StrengthDefinition.AttributeToCapture = UAuraAttributeSet::GetStrengthAttribute();
	StrengthDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthDefinition.bSnapshot = false;
	
	// 将要捕获的属性注册到计算器中
	RelevantAttributesToCapture.Add(VigorDefinition);
	RelevantAttributesToCapture.Add(StrengthDefinition);
}

// 实际计算基础数值（被系统调用）
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//BaseHealth + Vigor * VigorCoefficient + Strength * StrengthCoefficient + CharacterLevel * LevelCoefficient;
	static constexpr float BaseHealth = 90.f;
	static constexpr float VigorCoefficient = 10.f;
	static constexpr float StrengthCoefficient = 5.f;
	static constexpr float LevelCoefficient = 10.f;

	// 获取来源与目标的标签（用于按标签计算修正）
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	// 设置评估参数（将标签传入以便聚合器使用）
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	// 从捕获的属性中读取 Vigor 值（可能为 0）
	float Vigor = 0.0f;
	GetCapturedAttributeMagnitude(VigorDefinition, Spec, EvaluateParameters, Vigor);
	// 确保 Vigor 非负
	Vigor = FMath::Max(Vigor, 0.0f);
	
	float Strength = 0.0f;
	GetCapturedAttributeMagnitude(StrengthDefinition, Spec, EvaluateParameters, Strength);
	Strength = FMath::Max(Strength, 0.0f);
	
	// 从 EffectSpec 的上下文中获取实现了 ICombatInterface 的对象以读取等级
	// 这里做空指针检查，避免直接调用导致崩溃
	int32 CharacterLevel = 0;
	if (UObject* SourceObject = Spec.GetContext().GetSourceObject())
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(SourceObject))
		{
			CharacterLevel = CombatInterface->GetCharacterLevel();
		}
	}
	
	return BaseHealth + Vigor * VigorCoefficient + Strength * StrengthCoefficient + CharacterLevel * LevelCoefficient;
}
