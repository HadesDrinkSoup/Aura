// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_HealthRegen.generated.h"

/**
 * 战士 · 生命恢复
 * 公式：1.5 + 活力 × 0.18 + 等级 × 0.06（钳制：最低 0）
 * 生命回复偏高：活力既是生命上限的主属性又是回复的主系数，战士越堆活力越"耐耗"。
 *
 * 在系统里的角色：派生属性 HealthRegen 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 HealthRegen 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 语义是"每秒自动恢复的生命值"（见 AuraGamepalyTags.h 的 HealthRegen 注释），
 *    不是受击后的吸血/回血效果，也不参与战斗内即时结算；
 *  - 只捕获活力：回血与力量/智力/敏捷无关，所以纯力量 build 的战士回复会明显偏低——
 *    这是"活力=续航、力量=输出"的属性分工，调参时不要给力量加回复系数来补偿。
 *
 * 量纲：点/秒（L1 ≈ 3.72，L50 ≈ 14.76）；相对 2339 的满级血量只有约 0.6%/秒，
 * 属于"脱战/长线消耗"级回复，不是战斗内的主要生存来源。
 *
 * 调参：数值全部来自下面 3 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（仅活力，目标侧、非快照） */
	UMMC_Warrior_HealthRegen();

	/**
	 * 计算生命恢复 = 1.5 + 活力 × 0.18 + 等级 × 0.06，结果不低于 0。
	 * 由 GAS 在 GE 施加时调用一次，之后活力变化或等级提升再调用；实现必须是纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取活力并解析角色等级
	 * @return 每秒生命恢复量（>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 生命恢复：1.5 + 活力 × 0.18 + 等级 × 0.06
	// 生命回复偏高：活力既是生命上限的主属性又是回复的主系数。
	/** 基础生命恢复（点/秒）：没有任何活力/等级时的保底回复，让 L1 不会"零回复" */
	static constexpr float BaseHealthRegen = 1.5f;

	/** 活力系数：每 1 点活力 +0.18 点/秒，是回复的主权重（L50 活力 ≈ 76 贡献主体） */
	static constexpr float VigorCoefficient = 0.18f;

	/** 等级系数：每级 +0.06 点/秒（L50 = +3），保证不堆活力的 build 也有微量自然回复 */
	static constexpr float LevelCoefficient = 0.06f;
};
