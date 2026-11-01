// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_Armor.generated.h"

/**
 * 战士 · 护甲
 * 公式：5 + 活力 × 1.8 + 力量 × 1.3 + 等级 × 0.55（钳制：最低 0）
 * 护甲次高：活力 + 力量双加成，物理减伤核心（坦克的双系数更高，L50 约 331 vs 战士 221）。
 *
 * 在系统里的角色：派生属性 Armor 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 Armor 那条 Modifier 引用（Modifier Op = Add，
 *    Duration Policy = Infinite），职业 GE 由 DA_CharacterClassInfo 统一下发；
 *  - Infinite 下 GAS 会监听构造里登记的非快照捕获，活力/力量一变就自动重算并写回 Armor，
 *    所以装备/Buff/个体差异 GE 改动主属性后不需要手动刷新护甲；
 *  - 公式里没有任何随机或时间因素，服务端结算与客户端预测会得到同一数值。
 *
 * 量纲：护甲是"评分值"而不是减伤百分比（L1 ≈ 44，L50 ≈ 221）；设计上按 抗性/(抗性+常数)
 * 折算物理减伤，绝对值的领先在折算后会被压缩，调参时不要按线性减伤来理解。
 *
 * 调参：数值全部来自下面 4 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（活力 + 力量，目标侧、非快照） */
	UMMC_Warrior_Armor();

	/**
	 * 计算护甲 = 5 + 活力 × 1.8 + 力量 × 1.3 + 等级 × 0.55，结果不低于 0。
	 * 由 GAS 在 GE 施加时调用一次，之后每次被捕获属性变化再调用；实现必须是纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取捕获属性值并解析角色等级
	 * @return 护甲值（评分，>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 护甲：5 + 活力 × 1.8 + 力量 × 1.3 + 等级 × 0.55
	// 护甲次高：活力 + 力量双加成，物理减伤核心。
	/** 基础护甲（评分）：活力/力量/等级都为 0 时的保底值；角色 L1 就已有主属性，实际取不到 5 */
	static constexpr float BaseArmor = 5.f;

	/** 活力系数：每 1 点活力 +1.8 护甲，是护甲的主权重，让"堆活力"同时换血量和物理减伤 */
	static constexpr float VigorCoefficient = 1.8f;

	/** 力量系数：每 1 点力量 +1.3 护甲，物攻主属性顺带提供减伤，避免全堆力量时过于脆弱 */
	static constexpr float StrengthCoefficient = 1.3f;

	/** 等级系数：每级 +0.55 护甲（L50 = +27.5），与主属性解耦，保证任何 build 在后期都有基础减伤 */
	static constexpr float LevelCoefficient = 0.55f;
};
