// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MagicalAttack.generated.h"

/**
 * 战士 · 魔法攻击
 * 公式：2 + 智力 × 0.8 + 等级 × 0.3（钳制：最低 0）
 * 法攻偏低：智力是战士的次要属性，系数 0.8 只够让技能有一点点数值成长。
 *
 * 在系统里的角色：派生属性 MagicalAttack 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 MagicalAttack 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 基础项只有 2：战士的伤害主轴是 PhysicalAttack（力量 × 3.2），法攻只是"别为零"的补充，
 *    用于少数法术型技能或元素附带伤害；
 *  - 只捕获智力，力量/敏捷完全不参与，因此纯物理 build 的战士法攻会长期贴着个位数——
 *    这是有意的定位，避免战士同时吃力量与智力两个乘区而变成全能职业。
 *
 * 调参：数值全部来自下面 3 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_MagicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（仅智力，目标侧、非快照） */
	UMMC_Warrior_MagicalAttack();

	/**
	 * 计算魔法攻击 = 2 + 智力 × 0.8 + 等级 × 0.3，结果不低于 0。
	 * 由 GAS 在 GE 施加时及智力/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取智力与解析角色等级
	 * @return 魔法攻击力（>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 魔法攻击：2 + 智力 × 0.8 + 等级 × 0.3
	// 法攻偏低：智力是战士的次要属性，系数 0.8 只够让技能有一点点数值成长。
	/** 基础魔攻：2 点白板法攻，保证没有任何智力时法术类技能也有最小伤害底数 */
	static constexpr float BaseMagicalAttack = 2.f;

	/** 智力系数：每 1 点智力 +0.8 魔攻（L50 智力 22 约 +17.6），战士堆智力性价比明显低于力量 */
	static constexpr float IntelligenceCoefficient = 0.8f;

	/** 等级系数：每级 +0.3 魔攻（L50 = +15），让法攻随等级缓慢抬升，但始终低于物攻成长 */
	static constexpr float LevelCoefficient = 0.3f;
};
