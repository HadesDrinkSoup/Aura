// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MagicResistance.generated.h"

/**
 * 战士 · 魔法抗性
 * 公式：1 + 活力 × 1.6 + 智力 × 0.15 + 等级 × 1.1（钳制：最低 0）
 * 魔抗中等：活力主系数带魔抗，法师/坦克更高（坦克 L50 约 199.6，战士约 150.5）。
 *
 * 在系统里的角色：派生属性 MagicResistance 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 MagicResistance 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 与 Armor 是"同一套评分刻度"的两条防御线：护甲吃力量+Vigor 双系数更高，
 *    魔抗把权重放在活力、只给智力 0.15 的象征性系数，所以纯力量 build 的物抗强、魔抗弱，
 *    堆活力则两边都能稳住——这是战士"靠三维均衡抗伤"的核心设计；
 *  - 构造里登记了两个捕获（活力 + 智力），任一变化都会触发重算。
 *
 * 量纲：评分值而非百分比（L1 ≈ 21.9，L50 ≈ 150.5），与 Armor 一样由消费方按曲线折算减伤。
 *
 * 调参：数值全部来自下面 4 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（活力 + 智力，目标侧、非快照） */
	UMMC_Warrior_MagicResistance();

	/**
	 * 计算魔法抗性 = 1 + 活力 × 1.6 + 智力 × 0.15 + 等级 × 1.1，结果不低于 0。
	 * 由 GAS 在 GE 施加时及活力/智力/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取活力、智力并解析角色等级
	 * @return 魔法抗性值（评分，>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 魔法抗性：1 + 活力 × 1.6 + 智力 × 0.15 + 等级 × 1.1
	// 魔抗中等：活力主系数带魔抗，法师/坦克更高。
	/** 基础魔抗（评分）：白板 1 点底数，几乎可以忽略，仅用于表达"公式形状"而非实际防御 */
	static constexpr float BaseMagicResistance = 1.f;

	/** 活力系数：每 1 点活力 +1.6 魔抗，本公式的绝对主权重，让堆血同时变成法术减伤 */
	static constexpr float VigorCoefficient = 1.6f;

	/** 智力系数：每 1 点智力 +0.15 魔抗，象征性系数，避免智力对战士完全无防御收益 */
	static constexpr float IntelligenceCoefficient = 0.15f;

	/** 等级系数：每级 +1.1 魔抗（L50 = +55），与护甲的 0.55 形成对比：战士魔抗更依赖等级而非力量 */
	static constexpr float LevelCoefficient = 1.1f;
};
