// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "GameplayEffectAggregator.h"
#include "AuraMMCBase.generated.h"

/**
 * 自定义 Magnitude 计算的公共基类
 *
 * 统一处理所有职业 MMC 共用的样板逻辑：
 *  - 以「目标 + 非快照」方式登记要捕获的属性，保证派生属性随基础属性实时刷新
 *  - 安全读取捕获属性（未登记或读取失败时返回 0，并保证结果不为负）
 *  - 构建带来源/目标标签的评估参数（供属性聚合器按标签过滤）
 *  - 从 EffectContext 解析角色等级（SourceObject -> Instigator 兜底）
 *
 * 具体公式由 Elementalist / Warrior / Ranger 三个目录下的子类实现；
 * 在编辑器中把 GE 的 Modifier -> Calculation Class 指向对应职业的 MMC 即可。
 *
 * 注意：派生属性 GE 是施加给自身的，所以所有属性都从 Target 捕获。
 */
UCLASS(Abstract)
class AURA_API UAuraMMCBase : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UAuraMMCBase();

protected:
	/** 登记单个目标属性捕获（重复登记会被自动忽略） */
	void CaptureTargetAttribute(const FGameplayAttribute& Attribute);

	/** 登记两个目标属性捕获，等价于连续调用两次 CaptureTargetAttribute */
	void CaptureTargetAttributes(const FGameplayAttribute& FirstAttribute, const FGameplayAttribute& SecondAttribute);

	/** 在已登记的捕获列表中查找属性定义，找不到返回 nullptr */
	const FGameplayEffectAttributeCaptureDefinition* FindCaptureDefinition(const FGameplayAttribute& Attribute) const;

	/** 读取捕获属性数值；未登记/读取失败返回 0，并保证结果不小于 0 */
	float GetCapturedMagnitude(const FGameplayAttribute& Attribute,
		const FGameplayEffectSpec& Spec,
		const FAggregatorEvaluateParameters& EvaluationParameters) const;

	/** 构建带来源与目标标签的评估参数 */
	static FAggregatorEvaluateParameters BuildEvaluateParameters(const FGameplayEffectSpec& Spec);

	/** 解析施法者角色等级：优先 SourceObject，其次 Instigator，都解析不到时返回 0 */
	static int32 GetCharacterLevelFromSpec(const FGameplayEffectSpec& Spec);
};
