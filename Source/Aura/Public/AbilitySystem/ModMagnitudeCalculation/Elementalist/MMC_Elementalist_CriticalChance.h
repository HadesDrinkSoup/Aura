// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_CriticalChance.generated.h"

/**
 * 元素法师 · 暴击几率
 * 公式：4 + 智力 × 0.05 + 敏捷 × 0.04（钳制：0 ~ 75）
 * 暴击偏低：法系不靠爆发，智力只做温和加成，排在战士之后、坦克之前。
 *
 * 单位：百分比（0~75）。上限 75 是刻意的设计：即便后期装备/Buff 堆满，
 * 也永远保留 25% 的不暴击概率，避免"刀刀暴击"让伤害模型失真；
 * 下限 0 则是给未来可能出现的"减暴击"Debuff 兜底。
 *
 * 为什么这样设计：智力（0.05/点）略高于敏捷（0.04/点），意味着法系用主属性换暴击，
 * 但两个系数都很小，暴击收益主要来自装备/天赋而非升级。
 */
UCLASS()
class AURA_API UMMC_Elementalist_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：登记本公式的两个属性依赖（智力、敏捷），顺序与公式中的书写顺序一致 */
	UMMC_Elementalist_CriticalChance();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取智力与敏捷，
	 * 按百分比返回钳制在 0~75 之间的暴击几率。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 暴击几率：4 + 智力 × 0.05 + 敏捷 × 0.04
	// 暴击偏低：法系不靠爆发，智力只做温和加成，排在战士之后、坦克之前。
	/** 常数项 4%：不投属性时的保底暴击率，略高于坦克（3%），低于战士（5%）、游侠（8%） */
	static constexpr float BaseCriticalChance = 4.f;

	/** 智力 → 暴击：每 1 点智力 0.05%，法系的主属性折暴击路径（收益温和） */
	static constexpr float IntelligenceCoefficient = 0.05f;

	/** 敏捷 → 暴击：每 1 点敏捷 0.04%，法师的次要属性也顺带提供一点暴击 */
	static constexpr float AgilityCoefficient = 0.04f;
};
