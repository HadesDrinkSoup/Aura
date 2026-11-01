// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_AttackSpeed.generated.h"

/**
 * 元素法师 · 攻击速度
 * 公式：95 + 敏捷 × 0.15 + 等级 × 0.1（钳制：50 ~ 250）
 * 基础攻速最低，普攻不是主要输出手段。
 *
 * 单位：工程里没有独立的攻速单位，数值刻意贴着 100 量级（钳制区间也取 50~250），
 * 因此计算和显示都应把它当"相对基准攻速的百分比倍率"（100 ≈ 基准），而不是每秒攻击次数。
 *
 * 为什么这样设计：敏捷收益 0.15/点、等级仅 0.1/级，靠属性把攻速堆起来非常慢；
 * 普攻/技能频率主要由装备与 Buff 决定，这是"法师不靠平 A"的数值体现。
 */
UCLASS()
class AURA_API UMMC_Elementalist_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：只登记本公式唯一的属性依赖（敏捷）；等级不是属性，不通过捕获获取 */
	UMMC_Elementalist_AttackSpeed();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取敏捷与角色等级，
	 * 返回钳制在 50~250 之间的攻速面板值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 攻击速度：95 + 敏捷 × 0.15 + 等级 × 0.1
	// 基础攻速最低，普攻不是主要输出手段。
	/** 基础攻速 95：攻击频率的下限底子，比战士（100）、游侠（110）都低，仅高于坦克（90） */
	static constexpr float BaseAttackSpeed = 95.f;

	/** 敏捷 → 攻速：每 1 点敏捷 0.15 点攻速，是法师唯一的属性成长来源（收益远低于游侠） */
	static constexpr float AgilityCoefficient = 0.15f;

	/** 等级 → 攻速：每级 0.1 点，50 级也只有 5 点；升级几乎不改变攻击频率 */
	static constexpr float LevelCoefficient = 0.1f;
};
