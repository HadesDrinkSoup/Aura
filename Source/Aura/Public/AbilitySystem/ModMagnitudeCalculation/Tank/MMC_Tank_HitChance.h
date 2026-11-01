// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_HitChance.generated.h"

/**
 * 坦克 · 命中几率
 * 公式：79 + 敏捷 × 0.05 + 等级 × 0.12（钳制：0 ~ 100）
 * 命中最低：动作迟缓，靠站桩与范围技能弥补。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「命中几率」这一个 Modifier 以 Custom Calculation Class 引用。
 * 单位是百分点。基础值 79 已经是四职业最低，但仍刻意保持在 80 附近：
 * 命中的绝对值偏低会让战斗变成大量 Miss，坦克的「站桩」玩法会直接失效，
 * 所以成长空间压小（敏捷 0.05 / 级 0.12），下限则靠这个较高的基础值兜住。
 * 上限 100：命中超过 100% 没有意义，直接封顶。
 */
UCLASS()
class AURA_API UMMC_Tank_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：只登记敏捷——命中不吃力量/活力，坦克敏捷又只有 16% 投点，
	 * 所以命中成长主要靠等级项而不是属性；非快照捕获使敏捷变化后自动重算。
	 */
	UMMC_Tank_HitChance();

	/**
	 * 计算命中几率幅度：BaseHitChance + 敏捷 × 0.05 + 角色等级 × 0.12。
	 * 按 CurveTable 主属性推算，L1 ≈ 79.42、L50 ≈ 86.45：整条曲线几乎由等级项拉出来，
	 * 这样「不投敏捷也不至于打不中」，同时保留「四职业命中最低」的相对定位。
	 * @param Spec 正在结算的 GE Spec
	 * @return 命中几率（百分点），钳制在 0 ~ 100
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 命中几率：79 + 敏捷 × 0.05 + 等级 × 0.12
	// 命中最低：动作迟缓，靠站桩与范围技能弥补。
	/** 基础命中（百分点）：79，四职业最低（元素 80 / 战士 82 / 游侠 88）但绝对值仍很高 */
	static constexpr float BaseHitChance = 79.f;

	/** 每 1 点敏捷换取的命中（百分点/点）：0.05，四职业最低（游侠 0.18） */
	static constexpr float AgilityCoefficient = 0.05f;

	/** 每 1 角色等级附加的命中（百分点/级）：0.12，是坦克命中成长的主要来源，满级约 +6 */
	static constexpr float LevelCoefficient = 0.12f;
};
