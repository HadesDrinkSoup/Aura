// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_DodgeChance.generated.h"

/**
 * 游侠 · 闪避几率
 * 公式：6 + 敏捷 × 0.18（钳制：0 ~ 60）
 * 闪避核心职业：敏捷收益最高。
 *
 * 在系统中的位置：
 *  - 作为 GE_DerivedAttributes_Ranger 中 DodgeChance 这个 Modifier 的 Calculation Class，
 *    由 AAuraCharacterBase::InitDefaultAttributes 随职业派生属性 GE 施加；
 *  - 单属性公式：只看敏捷，不含等级项、也不含任何副属性，是游侠各公式里最简单的一条；
 *  - 返回闪避概率的百分比目标值（Modifier Op = Add），不是减伤比例。
 *
 * 量纲：百分比，上限 60 表示最多 60% 概率闪开攻击；钳制写在 MMC 内部，
 * 因为闪避是「靠一条属性一路堆」的曲线，必须在源头限制天花板。
 */
UCLASS()
class AURA_API UMMC_Ranger_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 登记闪避公式唯一依赖的属性：敏捷。
	 * 捕获源为 Target 且非快照——敏捷一变，Infinite 派生 GE 就会重算闪避并写回属性。
	 */
	UMMC_Ranger_DodgeChance();

	/**
	 * 计算游侠闪避几率的目标幅度：6 + 敏捷 × 0.18，并钳制到 0 ~ 60。
	 * 本公式只有敏捷一个变量，等级与其它主属性都不参与。
	 * @param Spec 触发本次求值的 GE Spec，敏捷从它携带的捕获数据里解析。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 闪避几率：6 + 敏捷 × 0.18
	// 闪避核心职业：敏捷收益最高。
	/** 基础闪避率（百分比）：6% 起手值，四职业最高，保证裸装游侠也有一点生存手段 */
	static constexpr float BaseDodgeChance = 6.f;

	/** 每 1 点敏捷贡献的闪避率（百分点）；0.18 为四职业最高，是「闪避核心职业」的依据 */
	static constexpr float AgilityCoefficient = 0.18f;
};
