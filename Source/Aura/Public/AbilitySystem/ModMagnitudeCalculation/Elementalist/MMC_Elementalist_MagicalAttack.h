// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MagicalAttack.generated.h"

/**
 * 元素法师 · 魔法攻击力
 * 公式：12 + 智力 × 3.4 + 等级 × 1.5（钳制：最低 0）
 * 核心输出：智力收益四职业最高。
 *
 * 单位：伤害点数，作为法术伤害的基础值参与伤害计算
 * （见 AuraGamepalyTags.h 中 Attributes.Derived.MagicalAttack 的说明，与 PhysicalAttack 同量纲）。
 *
 * 为什么这样设计：智力 3.4/点是四职业最高收益，等级 1.5/级在 50 级时贡献 75 点，
 * 也就是说"升级 + 堆智力"构成了这个职业几乎全部的输出成长，
 * 技能倍率/装备都只是在这个基数上做乘法。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MagicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：只登记智力一个属性捕获；等级在计算时从 EffectContext 解析 */
	UMMC_Elementalist_MagicalAttack();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取智力与角色等级，
	 * 返回不小于 0 的魔法攻击力面板值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 魔法攻击力：12 + 智力 × 3.4 + 等级 × 1.5
	// 核心输出：智力收益四职业最高。
	/** 常数项 12 点：1 级 0 智力时也有 13.5（含等级项）的基础法强，前期不至于完全没伤害 */
	static constexpr float BaseMagicalAttack = 12.f;

	/** 智力 → 法强：每 1 点智力 3.4 点法强，四职业中最高的属性收益，核心成长项 */
	static constexpr float IntelligenceCoefficient = 3.4f;

	/** 等级 → 法强：每级 1.5 点，50 级贡献 75 点，是除智力外最大的来源 */
	static constexpr float LevelCoefficient = 1.5f;
};
