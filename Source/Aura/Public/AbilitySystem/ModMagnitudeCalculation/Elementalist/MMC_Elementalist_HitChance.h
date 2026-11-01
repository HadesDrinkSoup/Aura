// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_HitChance.generated.h"

/**
 * 元素法师 · 命中几率
 * 公式：80 + 敏捷 × 0.06 + 智力 × 0.05 + 等级 × 0.1（钳制：0 ~ 100）
 * 命中偏低，需要装备或天赋补足。
 *
 * 单位：百分比（0~100，两端就是概率的数学定义域，不是可调的游戏性上限）。
 *
 * 为什么这样设计：基础值 80 说明"命中主要靠底子"，三个加成项都很小
 * （敏捷 0.06/点、智力 0.05/点、等级 0.1/级），因此想补命中只能靠装备/天赋，
 * 属性成长对命中率的贡献非常有限。
 */
UCLASS()
class AURA_API UMMC_Elementalist_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：登记公式用到的两个主属性（敏捷、智力）；等级从 EffectContext 解析 */
	UMMC_Elementalist_HitChance();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取敏捷、智力与角色等级，
	 * 按百分比返回钳制在 0~100 之间的命中几率。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 命中几率：80 + 敏捷 × 0.06 + 智力 × 0.05 + 等级 × 0.1
	// 命中偏低，需要装备或天赋补足。
	/** 常数项 80%：法师的命中底子，保证低等级时也不会因为属性为 0 而全程 Miss */
	static constexpr float BaseHitChance = 80.f;

	/** 敏捷 → 命中：每 1 点敏捷 0.06%，法师的主要命中属性（但与游侠的 0.18 相比很低） */
	static constexpr float AgilityCoefficient = 0.06f;

	/** 智力 → 命中：每 1 点智力 0.05%，顺带让法系主属性也提供一点命中 */
	static constexpr float IntelligenceCoefficient = 0.05f;

	/** 等级 → 命中：每级 0.1%，50 级约 5%，是仅次于常数项的第二大来源 */
	static constexpr float LevelCoefficient = 0.1f;
};
