// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_AttackSpeed.generated.h"

/**
 * 游侠 · 攻击速度
 * 公式：110 + 敏捷 × 0.45 + 等级 × 0.2（钳制：50 ~ 250）
 * 攻速最高，高频普攻与技能衔接是核心玩法。
 *
 * 在系统中的位置：
 *  - 作为 GE_DerivedAttributes_Ranger 中 AttackSpeed 这个 Modifier 的 Calculation Class，
 *    由 AAuraCharacterBase::InitDefaultAttributes 随职业派生属性 GE 一起施加；
 *  - 捕获为非快照，敏捷或等级变化都会让 GAS 重新调用本类并写回 AttackSpeed；
 *  - 返回的是「攻速目标值」（Modifier Op = Add），不是每秒攻击次数，也不是增量。
 *
 * 量纲：百分比式攻速，100 附近为常规水平。钳制写在 MMC 内部而不是 AttributeSet：
 * 派生 GE 是攻速的唯一来源，在这里夹住即可保证主属性怎么成长都不会算出离谱的攻速。
 */
UCLASS()
class AURA_API UMMC_Ranger_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 登记攻速公式唯一依赖的属性：敏捷。
	 * 游侠的攻速不吃等级以外的任何主属性，因此这里只需捕获一个；
	 * 等级不通过捕获读取，而是每次求值时从 GE 上下文解析。
	 */
	UMMC_Ranger_AttackSpeed();

	/**
	 * 计算游侠攻速的目标幅度：110 + 敏捷 × 0.45 + 等级 × 0.2，并钳制到 50 ~ 250。
	 * 主属性变化触发的每次重算都会走这里；等级解析不到时退化为只有敏捷项的数值。
	 * @param Spec 触发本次求值的 GE Spec，属性值与等级都从它解析。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 攻击速度：110 + 敏捷 × 0.45 + 等级 × 0.2
	// 攻速最高，高频普攻与技能衔接是核心玩法。
	/** 基础攻速（百分比基准）：110 是四职业中最高的一档，不投入敏捷时的起点 */
	static constexpr float BaseAttackSpeed = 110.f;

	/** 每 1 点敏捷贡献的攻速；0.45 为四职业最高，敏捷正是游侠的主属性 */
	static constexpr float AgilityCoefficient = 0.45f;
	/** 每 1 个角色等级贡献的攻速；增长较缓，保证攻速主要靠堆敏捷而不是靠升级 */
	static constexpr float LevelCoefficient = 0.2f;
};
