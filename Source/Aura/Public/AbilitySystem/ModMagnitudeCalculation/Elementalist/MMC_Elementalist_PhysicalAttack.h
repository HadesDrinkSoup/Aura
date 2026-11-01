// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_PhysicalAttack.generated.h"

/**
 * 元素法师 · 物理攻击力
 * 公式：3 + 力量 × 1 + 敏捷 × 0.3 + 等级 × 0.6（钳制：最低 0）
 * 几乎不依赖武器，物理攻击只作兜底输出。
 *
 * 单位：伤害点数，作为物理伤害的基础值参与伤害计算
 * （见 AuraGamepalyTags.h 中 Attributes.Derived.PhysicalAttack 的说明，与 MagicalAttack 同量纲）。
 *
 * 为什么这样设计：三个系数（力量 1、敏捷 0.3、等级 0.6）都是四职业垫底，
 * 常数项也只有 3。物理攻击对法师只是"没蓝时还能平 A"的兜底输出，
 * 而不是可以投入资源去堆的成长方向——这也是法系与物理系分化的关键。
 */
UCLASS()
class AURA_API UMMC_Elementalist_PhysicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：登记公式用到的两个主属性（力量、敏捷）；等级从 EffectContext 解析 */
	UMMC_Elementalist_PhysicalAttack();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取力量、敏捷与角色等级，
	 * 返回不小于 0 的物理攻击力面板值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 物理攻击力：3 + 力量 × 1 + 敏捷 × 0.3 + 等级 × 0.6
	// 几乎不依赖武器，物理攻击只作兜底输出。
	/** 常数项 3 点：极低的物理底子，1 级裸装时物理攻击只有 3.6（含等级项） */
	static constexpr float BasePhysicalAttack = 3.f;

	/** 力量 → 物攻：每 1 点力量 1 点物攻，系数只有战士（3.2）的三分之一 */
	static constexpr float StrengthCoefficient = 1.f;

	/** 敏捷 → 物攻：每 1 点敏捷 0.3 点物攻，法师的次要属性对物攻帮助有限 */
	static constexpr float AgilityCoefficient = 0.3f;

	/** 等级 → 物攻：每级 0.6 点，50 级也只有 30 点，升级不会让法师变成物理输出 */
	static constexpr float LevelCoefficient = 0.6f;
};
