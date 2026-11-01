// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_MagicalAttack.generated.h"

/**
 * 坦克 · 魔法攻击力
 * 公式：1 + 智力 × 0.5 + 等级 × 0.1（钳制：最低 0）
 * 几乎零法强，对抗法术靠抗性而不是伤害。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「魔法攻击力」这一个 Modifier 以 Custom Calculation Class 引用。
 * 保留这条极小的公式（按 CurveTable 推算 L1 ≈ 3.1、L50 ≈ 17）而不是直接写 0，
 * 是为了让「带法强缩放的技能/道具」在坦克身上仍有合法数值，不至于除零或显示异常，
 * 但数值上不构成任何输出方向。
 */
UCLASS()
class AURA_API UMMC_Tank_MagicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：只登记智力。
	 * 法强是坦克最不关心的属性（智力只占主属性预算 12%），因此这里只捕获单一属性，
	 * 捕获越少，Infinite 派生 GE 的依赖监听与重算路径越短。
	 */
	UMMC_Tank_MagicalAttack();

	/**
	 * 计算魔法攻击力幅度：BaseMagicalAttack + 智力 × 0.5 + 角色等级 × 0.1。
	 * 智力系数 0.5 与等级系数 0.1 都是四职业最低（元素 3.4 / 1.5），
	 * 所以即便把智力堆满，坦克的法强也涨不动——这是设计上的硬性弱项。
	 * @param Spec 正在结算的 GE Spec
	 * @return 魔法攻击力（点），保底 0
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 魔法攻击力：1 + 智力 × 0.5 + 等级 × 0.1
	// 几乎零法强，对抗法术靠抗性而不是伤害。
	/** 基础法强（点）：1，四职业最低（战士 2 / 游侠 4 / 元素 12） */
	static constexpr float BaseMagicalAttack = 1.f;

	/** 每 1 点智力换取的魔攻（点/点）：0.5，四职业最低（元素 3.4 / 游侠 1.5 / 战士 0.8） */
	static constexpr float IntelligenceCoefficient = 0.5f;

	/** 每 1 角色等级附加的魔攻（点/级）：0.1，满级仅 +5，四职业最低（元素 1.5） */
	static constexpr float LevelCoefficient = 0.1f;
};
