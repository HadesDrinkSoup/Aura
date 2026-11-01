// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_CriticalChance.generated.h"

/**
 * 游侠 · 暴击几率
 * 公式：8 + 敏捷 × 0.2 + 智力 × 0.03（钳制：0 ~ 75）
 * 暴击核心职业：敏捷收益四职业最高。
 *
 * 在系统中的位置：
 *  - 作为 GE_DerivedAttributes_Ranger 中 CriticalChance 这个 Modifier 的 Calculation Class，
 *    由 AAuraCharacterBase::InitDefaultAttributes 随职业派生属性 GE 施加；
 *  - 公式没有等级项：暴击只认敏捷（主）与智力（零头），升级不会直接白送暴击；
 *  - 返回的是暴击概率的百分比目标值（Modifier Op = Add），不是暴击倍率。
 *
 * 量纲：百分比，上限 75 表示最多 75% 概率出暴击；钳制留在 MMC 内部，
 * 保证单靠主属性成长不会突破设计上限。
 */
UCLASS()
class AURA_API UMMC_Ranger_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 登记暴击公式的两个输入属性：敏捷（主收益）、智力（小额补贴）。
	 * 两者以「目标 + 非快照」方式捕获，主属性变化时 Infinite 派生 GE 会自动重算暴击。
	 */
	UMMC_Ranger_CriticalChance();

	/**
	 * 计算游侠暴击几率的目标幅度：8 + 敏捷 × 0.2 + 智力 × 0.03，并钳制到 0 ~ 75。
	 * 本公式不含等级项，等级变化不会影响暴击数值。
	 * @param Spec 触发本次求值的 GE Spec，属性值从它携带的捕获数据里解析。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 暴击几率：8 + 敏捷 × 0.2 + 智力 × 0.03
	// 暴击核心职业：敏捷收益四职业最高。
	/** 基础暴击率（百分比）：8% 的起手值，四职业里最高的一档，不投入属性也有基本爆发 */
	static constexpr float BaseCriticalChance = 8.f;

	/** 每 1 点敏捷贡献的暴击率（百分点）；0.2 为四职业最高，是游侠「暴击核心」的主要来源 */
	static constexpr float AgilityCoefficient = 0.2f;
	/** 每 1 点智力贡献的暴击率（百分点）；0.03 只是象征性补贴，不构成第二成长曲线 */
	static constexpr float IntelligenceCoefficient = 0.03f;
};
