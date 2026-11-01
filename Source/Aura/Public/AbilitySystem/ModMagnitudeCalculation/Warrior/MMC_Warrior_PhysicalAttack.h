// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_PhysicalAttack.generated.h"

/**
 * 战士 · 物理攻击
 * 公式：12 + 力量 × 3.2 + 敏捷 × 0.4 + 等级 × 1.5（钳制：最低 0）
 * 物攻是战士的伤害主轴：力量系数 3.2 为本职业最高乘区，等级项 1.5 保证裸装也有成长。
 *
 * 在系统里的角色：派生属性 PhysicalAttack 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 PhysicalAttack 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 力量主加成、敏捷只给 0.4 的补充，所以纯粹的敏捷流战士物攻很低；
 *    与 CriticalChance（敏捷 0.12 / 力量 0.05）配合，形成"力量=单次伤害、敏捷=暴击与攻速"的
 *    两条分支，但战士的伤害重心明确压在力量这一侧；
 *  - 等级项 1.5 是本职业"基础伤害随等级长"的主要来源，调低它会直接影响战士前中期强度；
 *  - 两个捕获（力量 + 敏捷）都登记，任一变化都会触发重算。
 *
 * 量纲：伤害点数（L1 ≈ 58.3，L50 ≈ 313.8），不是百分比。
 *
 * 调参：数值全部来自下面 4 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_PhysicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（力量 + 敏捷，目标侧、非快照） */
	UMMC_Warrior_PhysicalAttack();

	/**
	 * 计算物理攻击 = 12 + 力量 × 3.2 + 敏捷 × 0.4 + 等级 × 1.5，结果不低于 0。
	 * 由 GAS 在 GE 施加时及力量/敏捷/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取力量、敏捷并解析角色等级
	 * @return 物理攻击力（>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 物理攻击：12 + 力量 × 3.2 + 敏捷 × 0.4 + 等级 × 1.5
	// 物攻是战士的伤害主轴：力量系数 3.2 为本职业最高乘区。
	/** 基础物攻：12 点白板攻击，保证 L1 裸装、无力量加成时也能打出有效伤害 */
	static constexpr float BasePhysicalAttack = 12.f;

	/** 力量系数：每 1 点力量 +3.2 物攻，全职业最高档，是战士伤害的绝对主轴 */
	static constexpr float StrengthCoefficient = 3.2f;

	/** 敏捷系数：每 1 点敏捷 +0.4 物攻，仅为分支 build 留口子，让敏捷流不至于毫无伤害 */
	static constexpr float AgilityCoefficient = 0.4f;

	/** 等级系数：每级 +1.5 物攻（L50 = +75），与主属性解耦的基础成长，决定前中期强度下限 */
	static constexpr float LevelCoefficient = 1.5f;
};
