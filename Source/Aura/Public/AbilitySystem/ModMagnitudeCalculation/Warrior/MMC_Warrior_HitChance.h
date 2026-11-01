// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_HitChance.generated.h"

/**
 * 战士 · 命中几率
 * 公式：82 + 敏捷 × 0.08 + 等级 × 0.15（钳制：0 ~ 100）
 * 命中成长空间最小：基础 82% 已很高，敏捷系数只有 0.08，满级约 92.6%。
 *
 * 在系统里的角色：派生属性 HitChance 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 HitChance 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 单位是百分比（82 表示 82%），与闪避/格挡等防御属性按同一刻度对拼；
 *  - 只捕获敏捷且系数极低，本职业几乎不可能靠属性把命中堆满：未命中率的设计空间
 *    留给 Buff/Debuff 的 Add 叠加，本公式只负责给出"职业基线"；
 *  - 上限钳在 100 是硬性语义要求——超过 100% 的命中在结算里没有意义，还会让后续
 *    减命中的 Debuff 被"隐藏的溢出值"吃掉一部分效果。
 *
 * 调参：数值全部来自下面 3 个 static constexpr（钳制上下限写死在 .cpp 里），改完需重新编译。
 */
UCLASS()
class AURA_API UMMC_Warrior_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（仅敏捷，目标侧、非快照） */
	UMMC_Warrior_HitChance();

	/**
	 * 计算命中几率 = 82 + 敏捷 × 0.08 + 等级 × 0.15，结果钳在 [0, 100]。
	 * 由 GAS 在 GE 施加时及敏捷/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取敏捷与解析角色等级
	 * @return 命中几率百分比（0 ~ 100）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 命中几率：82 + 敏捷 × 0.08 + 等级 × 0.15
	// 命中成长空间最小：基础 82% 已很高，敏捷系数只有 0.08。
	/** 基础命中率（%）：82% 的近战职业基线，剩下 18% 的未命中需要靠敏捷/等级/装备补 */
	static constexpr float BaseHitChance = 82.f;

	/** 敏捷系数：每 1 点敏捷 +0.08%（L50 敏捷 39 仅 +3.12%），命中不是敏捷的主要用途 */
	static constexpr float AgilityCoefficient = 0.08f;

	/** 等级系数：每级 +0.15%（L50 = +7.5%），是战士命中成长的主要来源，保证后期手感稳定 */
	static constexpr float LevelCoefficient = 0.15f;
};
