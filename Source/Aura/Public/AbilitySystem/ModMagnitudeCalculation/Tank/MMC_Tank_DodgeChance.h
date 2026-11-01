// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_DodgeChance.generated.h"

/**
 * 坦克 · 闪避几率
 * 公式：1 + 敏捷 × 0.06（钳制：0 ~ 60）
 * 闪避最低：不靠走位，靠血量和护甲硬吃。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「闪避几率」这一个 Modifier 以
 * Custom Calculation Class 引用；该 GE 是 Infinite，所以敏捷一变这里就会重算。
 */
UCLASS()
class AURA_API UMMC_Tank_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：登记本公式唯一依赖的基础属性——敏捷。
	 * 捕获清单必须在构造期定好（GAS 在 GE 结算前就固化捕获表），
	 * 漏登记的属性在 Calculate 里读不到值，基类只会返回 0 并打 Warning，闪避会退化成「只剩基础 1」。
	 */
	UMMC_Tank_DodgeChance();

	/**
	 * 计算闪避几率幅度：1 + 敏捷 × 0.06。
	 * 调用时机：派生 GE 施加瞬间算一次，之后每次敏捷聚合值变化由 GAS 再次调用（非快照捕获）。
	 * @param Spec 正在结算的 GE Spec；本公式不取等级，只用 Spec 里的来源/目标标签做评估参数
	 * @return 闪避几率，单位「百分点」，钳制在 0 ~ 60
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 闪避几率：1 + 敏捷 × 0.06
	// 闪避最低：不靠走位，靠血量和护甲硬吃。
	/** 基础闪避（百分点）：敏捷为 0 时的底值，保留 1 让极端配装也不至于 0% */
	static constexpr float BaseDodgeChance = 1.f;

	/** 每 1 点敏捷换取的闪避（百分点/点）：0.06 是四职业里最低的敏捷闪避收益 */
	static constexpr float AgilityCoefficient = 0.06f;
};
