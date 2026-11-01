// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MaxHealth.generated.h"

/**
 * 战士 · 最大生命值
 * 公式：140 + 活力 × 15 + 力量 × 9 + 等级 × 15（钳制：最低 1）
 * 血量偏高（L50 约 2339，仅次于坦克的约 2962）：活力与力量双收益，是前排承伤的核心。
 *
 * 在系统里的角色：派生属性 MaxHealth 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 MaxHealth 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 这是全表系数最大的一条公式：活力 ×15 是四职业通用主权重，力量 ×9 是战士的额外收益，
 *    再加上等级 ×15 的大项，使战士的容错明显高于游侠/法师，但仍低于专职坦克；
 *  - 两个属性都要登记捕获（活力 + 力量），否则单堆力量/单堆活力时血量不会刷新，
 *    会出现"升了属性但血条不变"的问题；
 *  - 血上限变化后由 UAuraAttributeSet 的 OnMaxHealthChanged/复制流程接力，
 *    本类只负责"算出该是多少"，不做任何血量同步或治疗。
 *
 * 量纲：生命点数（L1 ≈ 452，L50 ≈ 2339），不是百分比。
 *
 * 调参：数值全部来自下面 4 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_MaxHealth : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（活力 + 力量，目标侧、非快照） */
	UMMC_Warrior_MaxHealth();

	/**
	 * 计算最大生命值 = 140 + 活力 × 15 + 力量 × 9 + 等级 × 15，结果不低于 1。
	 * 由 GAS 在 GE 施加时及活力/力量/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取活力、力量并解析角色等级
	 * @return 生命上限（>= 1）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 最大生命值：140 + 活力 × 15 + 力量 × 9 + 等级 × 15
	// 血量偏高：活力与力量双收益，是前排承伤的核心。
	/** 基础生命（点）：140 的白板血量，约等于 L1 血量的三成，保证裸装也有承伤能力 */
	static constexpr float BaseHealth = 140.f;

	/** 活力系数：每 1 点活力 +15 生命，本表最高权重，活力是战士血量增长的绝对主轴 */
	static constexpr float VigorCoefficient = 15.f;

	/** 力量系数：每 1 点力量 +9 生命，输出主属性附带生存，让纯力量 build 不至于太脆 */
	static constexpr float StrengthCoefficient = 9.f;

	/** 等级系数：每级 +15 生命（L50 = +750），后期血量增长的最大单项，保证满级下限 */
	static constexpr float LevelCoefficient = 15.f;
};
