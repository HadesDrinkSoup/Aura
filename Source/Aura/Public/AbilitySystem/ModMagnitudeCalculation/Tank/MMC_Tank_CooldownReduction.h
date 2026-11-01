// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_CooldownReduction.generated.h"

/**
 * 坦克 · 冷却缩减
 * 公式：0 + 智力 × 0.06 + 等级 × 0.02（钳制：0 ~ 40）
 * 冷却缩减最低：技能节奏最慢。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「冷却缩减」这一个 Modifier 以 Custom Calculation Class 引用。
 * 单位是百分点（设计口径：40 表示冷却时间最多缩短 40%，压到 60%）；
 * 坦克智力只占主属性预算的 12%，所以这里基础值为 0、系数也是四职业最低——
 * 冷却缩减对坦克是「顺手长的附属项」，不是玩法核心。
 */
UCLASS()
class AURA_API UMMC_Tank_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：只登记智力。
	 * 坦克的 CD 缩减完全由智力驱动，而智力又是四主属性里投点最少的一项（12%），
	 * 这决定了它在同类属性里几乎永远垫底；非快照捕获使智力变化后自动重算。
	 */
	UMMC_Tank_CooldownReduction();

	/**
	 * 计算冷却缩减幅度：BaseCooldownReduction + 智力 × 0.06 + 角色等级 × 0.02。
	 * 0.02/级 的等级项意味着满级也只多 1 个百分点，成长几乎全靠智力投点；
	 * 按 CurveTable 主属性推算，L1 ≈ 0.26、L50 ≈ 2.32，离 40 的硬上限极远。
	 * @param Spec 正在结算的 GE Spec
	 * @return 冷却缩减（百分点），钳制在 0 ~ 40
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 冷却缩减：0 + 智力 × 0.06 + 等级 × 0.02
	// 冷却缩减最低：技能节奏最慢。
	/** 基础冷却缩减（百分点）：为 0，坦克没有任何「白送」的缩减，全部来自智力投点 */
	static constexpr float BaseCooldownReduction = 0.f;

	/** 每 1 点智力换取的冷却缩减（百分点/点）：0.06，四职业最低（元素 0.25 / 游侠 0.1 / 战士 0.08） */
	static constexpr float IntelligenceCoefficient = 0.06f;

	/** 每 1 角色等级附加的冷却缩减（百分点/级）：0.02，满级仅约 1 个百分点 */
	static constexpr float LevelCoefficient = 0.02f;
};
