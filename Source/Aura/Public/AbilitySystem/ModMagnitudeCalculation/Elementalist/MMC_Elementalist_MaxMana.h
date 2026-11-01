// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MaxMana.generated.h"

/**
 * 元素法师 · 最大法力值
 * 公式：120 + 智力 × 15 + 活力 × 2 + 等级 × 9（钳制：最低 0）
 * 法力池最大，智力收益最高，支撑长时间施法。
 *
 * 单位：法力点数。最低 0（不需要像 MaxHealth 那样保底 1）：法力为 0 只是"不能放技能"，
 * 不会导致死亡或除零；但下限仍必须存在——若 MaxMana 为负，
 * UAuraAttributeSet::PreAttributeChange 里 Clamp(Mana, 0, MaxMana) 的上下界会倒置。
 *
 * 与本属性相关的连锁行为：写入 MaxMana 时会命中 PostAttributeChange，
 * 由它把当前 Mana 重新钳到新上限（换装导致上限下降时不会剩出"超上限"的法力）。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MaxMana : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：登记公式用到的两个主属性（智力、活力）；等级从 EffectContext 解析 */
	UMMC_Elementalist_MaxMana();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取智力、活力与角色等级，
	 * 返回不小于 0 的最大法力值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 最大法力值：120 + 智力 × 15 + 活力 × 2 + 等级 × 9
	// 法力池最大，智力收益最高，支撑长时间施法。
	/** 常数项 120 点：四职业最高的法力底子，1 级就有 129 点可用 */
	static constexpr float BaseMana = 120.f;

	/** 智力 → 法力：每 1 点智力 15 点法力，全项目最高的单项属性收益，是"法力池最大"的核心 */
	static constexpr float IntelligenceCoefficient = 15.f;

	/** 活力 → 法力：每 1 点活力 2 点法力，法师的次要属性也顺带加一点池子 */
	static constexpr float VigorCoefficient = 2.f;

	/** 等级 → 法力：每级 9 点，50 级贡献 450 点，等级是除智力外最大的法力来源 */
	static constexpr float LevelCoefficient = 9.f;
};
