// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MaxMana.generated.h"

/**
 * 战士 · 最大法力值
 * 公式：25 + 智力 × 3.5 + 活力 × 1 + 等级 × 2.5（钳制：最低 0）
 * 法力池偏小（L50 约 284，坦克约 169 更小）：智力主加成，活力只有象征性补一点。
 *
 * 在系统里的角色：派生属性 MaxMana 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 MaxMana 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 智力系数 3.5 是池子的主来源，活力系数 1 只是让堆血的战士"顺带有一口蓝"，
 *    避免出现"血量很厚但一辈子放不出一个技能"的极端 build；
 *  - 两个属性都必须登记捕获：单独改智力（装备/天赋）或单独改活力都要触发池子重算；
 *  - 池子变化后由 UAuraAttributeSet 的变更委托/复制流程处理，本类只算上限值。
 *
 * 量纲：法力点数（L1 ≈ 53.5，L50 ≈ 284），不是百分比。
 *
 * 调参：数值全部来自下面 4 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_MaxMana : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（智力 + 活力，目标侧、非快照） */
	UMMC_Warrior_MaxMana();

	/**
	 * 计算最大法力值 = 25 + 智力 × 3.5 + 活力 × 1 + 等级 × 2.5，结果不低于 0。
	 * 由 GAS 在 GE 施加时及智力/活力/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取智力、活力并解析角色等级
	 * @return 法力上限（>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 最大法力值：25 + 智力 × 3.5 + 活力 × 1 + 等级 × 2.5
	// 法力池偏小：智力主加成，活力只有象征性补一点。
	/** 基础法力（点）：25 点的白板池子，够放一两次低耗技能，不足以支撑法系循环 */
	static constexpr float BaseMana = 25.f;

	/** 智力系数：每 1 点智力 +3.5 法力，本公式的主权重（L50 智力 22 ≈ +77） */
	static constexpr float IntelligenceCoefficient = 3.5f;

	/** 活力系数：每 1 点活力 +1 法力（L50 活力 ≈ 76 → +76），战士血量成长顺带带来的额外蓝量 */
	static constexpr float VigorCoefficient = 1.f;

	/** 等级系数：每级 +2.5 法力（L50 = +125），后期池子的主要增量 */
	static constexpr float LevelCoefficient = 2.5f;
};
