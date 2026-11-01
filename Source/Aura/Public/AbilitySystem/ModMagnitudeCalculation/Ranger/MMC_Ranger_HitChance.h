// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_HitChance.generated.h"

/**
 * 游侠 · 命中几率
 * 公式：88 + 敏捷 × 0.18 + 等级 × 0.2（钳制：0 ~ 100）
 * 命中很高，天生准。
 *
 * 在系统中的位置：
 *  - 作为 GE_DerivedAttributes_Ranger 中 HitChance 这个 Modifier 的 Calculation Class，
 *    由 AAuraCharacterBase::InitDefaultAttributes 随职业派生属性 GE 施加；
 *  - 起手就有 88% 的高基础命中，再加敏捷与等级：游侠几乎不会 miss，这是职业核心手感；
 *  - 返回命中概率的百分比目标值（Modifier Op = Add），与敌方闪避/格挡的对抗在命中判定阶段处理，
 *    不在本 MMC 内做，本 MMC 只负责产出己方命中率。
 *
 * 量纲：百分比，上限 100 表示必定命中；钳制写在 MMC 内部，
 * 让「高命中」始终是游侠的稳定特征而不会冲过 100% 的无意义区间。
 */
UCLASS()
class AURA_API UMMC_Ranger_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 登记命中公式的主属性输入：敏捷。
	 * 目标捕获 + 非快照：敏捷变化会让 Infinite 派生 GE 自动重算命中率。
	 */
	UMMC_Ranger_HitChance();

	/**
	 * 计算游侠命中几率的目标幅度：88 + 敏捷 × 0.18 + 等级 × 0.2，并钳制到 0 ~ 100。
	 * @param Spec 触发本次求值的 GE Spec，敏捷从捕获数据取，等级从 Spec 的来源角色解析。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 命中几率：88 + 敏捷 × 0.18 + 等级 × 0.2
	// 命中很高，天生准。
	/** 基础命中率（百分比）：88% 的高起手，直接把游侠与「需要堆命中」的职业区分开 */
	static constexpr float BaseHitChance = 88.f;

	/** 每 1 点敏捷贡献的命中率（百分点）；0.18 与闪避同档，敏捷同时喂命中与闪避两端 */
	static constexpr float AgilityCoefficient = 0.18f;
	/** 每 1 级贡献的命中率（百分点）；0.2 让等级稳步把命中推向满值，呼应"几乎不会 miss" */
	static constexpr float LevelCoefficient = 0.2f;
};
