// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "Interface/CombatInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogAuraMMC, Log, All);

namespace
{
	/** 尝试把 UObject 解析为可提供角色等级的对象 */
	int32 ResolveCharacterLevel(UObject* Candidate)
	{
		if (!Candidate)
		{
			return 0;
		}

		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Candidate))
		{
			return CombatInterface->GetCharacterLevel();
		}

		return 0;
	}
}

UAuraMMCBase::UAuraMMCBase()
{
	// 所有职业 MMC 都从「目标」捕获属性且不做快照，因此这里不需要额外配置。
	// 具体要捕获哪些属性，由各子类在构造函数中通过 CaptureTargetAttribute(s) 登记。
}

void UAuraMMCBase::CaptureTargetAttribute(const FGameplayAttribute& Attribute)
{
	if (!Attribute.IsValid())
	{
		UE_LOG(LogAuraMMC, Warning, TEXT("%s: 尝试捕获无效属性，已忽略"), *GetClass()->GetName());
		return;
	}

	// 同一个属性只登记一次，避免聚合器重复计算
	if (FindCaptureDefinition(Attribute))
	{
		return;
	}

	RelevantAttributesToCapture.Emplace(Attribute, EGameplayEffectAttributeCaptureSource::Target, false);
}

void UAuraMMCBase::CaptureTargetAttributes(const FGameplayAttribute& FirstAttribute, const FGameplayAttribute& SecondAttribute)
{
	CaptureTargetAttribute(FirstAttribute);
	CaptureTargetAttribute(SecondAttribute);
}

const FGameplayEffectAttributeCaptureDefinition* UAuraMMCBase::FindCaptureDefinition(const FGameplayAttribute& Attribute) const
{
	return RelevantAttributesToCapture.FindByPredicate(
		[&Attribute](const FGameplayEffectAttributeCaptureDefinition& Definition)
		{
			return Definition.AttributeToCapture == Attribute;
		});
}

float UAuraMMCBase::GetCapturedMagnitude(const FGameplayAttribute& Attribute,
	const FGameplayEffectSpec& Spec,
	const FAggregatorEvaluateParameters& EvaluationParameters) const
{
	const FGameplayEffectAttributeCaptureDefinition* Definition = FindCaptureDefinition(Attribute);
	if (!Definition)
	{
		// 属于代码配置错误：子类忘记在构造函数中登记该属性
		UE_LOG(LogAuraMMC, Warning, TEXT("%s: 属性 %s 未登记到 RelevantAttributesToCapture，按 0 处理"),
			*GetClass()->GetName(), *Attribute.GetName());
		return 0.f;
	}

	float Magnitude = 0.f;
	if (!GetCapturedAttributeMagnitude(*Definition, Spec, EvaluationParameters, Magnitude))
	{
		return 0.f;
	}

	// 基础属性按「非负」处理，避免负值导致派生属性出现异常
	return FMath::Max(Magnitude, 0.f);
}

FAggregatorEvaluateParameters UAuraMMCBase::BuildEvaluateParameters(const FGameplayEffectSpec& Spec)
{
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	return EvaluationParameters;
}

int32 UAuraMMCBase::GetCharacterLevelFromSpec(const FGameplayEffectSpec& Spec)
{
	const FGameplayEffectContext* EffectContext = Spec.GetContext().Get();
	if (!EffectContext)
	{
		return 0;
	}

	// 默认从 SourceObject 解析（InitDefaultAttributes 中会把角色自身设为 SourceObject）
	if (const int32 SourceObjectLevel = ResolveCharacterLevel(EffectContext->GetSourceObject()))
	{
		return SourceObjectLevel;
	}

	// 兜底：某些直接由技能/系统施加的 GE 只设置了 Instigator
	return ResolveCharacterLevel(EffectContext->GetInstigator());
}
