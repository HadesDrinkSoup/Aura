// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_HealthRegen.generated.h"

/**
 * 游侠 · 生命恢复
 * 公式：0.8 + 体质 × 0.1 + 等级 × 0.04（兜底：结果不低于 0）
 * 恢复能力一般，靠等级线性托底。
 *
 * 在系统中的位置：
 *  - 作为 GE_DerivedAttributes_Ranger 中 HealthRegen 这个 Modifier 的 Calculation Class，
 *    由 AAuraCharacterBase::InitDefaultAttributes 随职业派生属性 GE 施加给角色自身；
 *  - 只有一个主属性输入（体质），但额外吃等级：等级项让低投入角色也随进度缓慢回血；
 *  - 返回的是「每秒回血值」（Modifier Op = Add），不是百分比、也不是本次回血量。
 *
 * 量纲：生命/秒，数值都小于 1，靠高频 Tick 累积；公式本身无上限，
 * 只做不低于 0 的兜底，防止异常负值把角色血量往下拖。
 */
UCLASS()
class AURA_API UMMC_Ranger_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 登记生命恢复公式的主属性输入：体质。
	 * 以「目标 + 非快照」捕获，体质提升时 Infinite 派生 GE 会自动重算回血速度。
	 */
	UMMC_Ranger_HealthRegen();

	/**
	 * 计算游侠生命恢复的目标幅度：0.8 + 体质 × 0.1 + 等级 × 0.04，结果不低于 0。
	 * @param Spec 触发本次求值的 GE Spec，体质从捕获数据取，等级从 Spec 的来源角色解析。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 生命恢复：0.8 + 体质 × 0.1 + 等级 × 0.04
	// 恢复能力一般，靠等级线性托底。
	/** 基础回血（生命/秒）：0.8 表示脱离战斗后每秒回 0.8 点血，是「脱战能慢慢回满」的底子 */
	static constexpr float BaseHealthRegen = 0.8f;

	/** 每 1 点体质贡献的回血（生命/秒）；0.1 让体质成为唯一能主动堆高的恢复来源 */
	static constexpr float VigorCoefficient = 0.1f;
	/** 每 1 级贡献的回血（生命/秒）；0.04 是等级带来的被动托底，不堆体质也有缓慢成长 */
	static constexpr float LevelCoefficient = 0.04f;
};
