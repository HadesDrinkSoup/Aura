// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_CooldownReduction.generated.h"

/**
 * 游侠 · 冷却缩减
 * 公式：1 + 敏捷 × 0.1 + 智力 × 0.05（钳制：0 ~ 40）
 * 冷却缩减一般，主要来自敏捷。
 *
 * 在系统中的位置：
 *  - 作为 GE_DerivedAttributes_Ranger 中 CooldownReduction 这个 Modifier 的 Calculation Class，
 *    由 AAuraCharacterBase::InitDefaultAttributes 随职业派生属性 GE 施加给角色自身；
 *  - 与其它属性不同，本公式没有等级项：冷却缩减只随敏捷/智力成长，等级不会带来任何缩减；
 *  - 返回的是「冷却缩减百分比目标值」（Modifier Op = Add），不是剩余冷却，也不是秒数。
 *
 * 量纲：百分比，40 表示最多缩短 40% 冷却；上下限在 MMC 内部钳死，
 * 保证主属性怎么堆都不会算出超过 40% 的缩减。
 */
UCLASS()
class AURA_API UMMC_Ranger_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 登记冷却缩减公式的两个输入属性：敏捷、智力。
	 * 敏捷是主收益，智力只给很小的零头；两者都按「目标 + 非快照」捕获，
	 * 这样主属性变化时 Infinite 派生 GE 会自动重算并写回 CooldownReduction。
	 */
	UMMC_Ranger_CooldownReduction();

	/**
	 * 计算游侠冷却缩减的目标幅度：1 + 敏捷 × 0.1 + 智力 × 0.05，并钳制到 0 ~ 40。
	 * 注意本公式不含等级项，等级变化不会触发数值变化；主属性变化触发的每次重算都会走这里。
	 * @param Spec 触发本次求值的 GE Spec，属性值从它携带的捕获数据里解析。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 冷却缩减：1 + 敏捷 × 0.1 + 智力 × 0.05
	// 冷却缩减一般，主要来自敏捷。
	/** 基础冷却缩减（百分比）：1% 的起手值，避免 0 级角色完全没有技能节奏收益 */
	static constexpr float BaseCooldownReduction = 1.f;

	/** 每 1 点敏捷贡献的冷却缩减（百分点）；游侠的主属性，是冷却缩减的主要来源 */
	static constexpr float AgilityCoefficient = 0.1f;
	/** 每 1 点智力贡献的冷却缩减（百分点）；仅为敏捷的一半，只是顺带的加成 */
	static constexpr float IntelligenceCoefficient = 0.05f;
};
