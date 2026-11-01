// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_DodgeChance.generated.h"

/**
 * 元素法师 · 闪避几率
 * 公式：2 + 敏捷 × 0.08（钳制：0 ~ 60）
 * 闪避成长最差，不靠敏捷生存。
 *
 * 单位：百分比（0~60）。上限 60 让闪避永远做不到"完全免伤"，
 * 避免高闪避角色在 PVE 里无解；下限 0 只是概率定义域的自然边界。
 *
 * 为什么这样设计：系数仅 0.08/点敏捷，而且公式里**没有等级项**——
 * 升级不会让法师变得更难打中，闪避只随敏捷缓慢增长，是四职业里最平的曲线。
 */
UCLASS()
class AURA_API UMMC_Elementalist_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：只登记敏捷一个属性捕获；本公式不含等级项，也不读取角色等级 */
	UMMC_Elementalist_DodgeChance();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取敏捷，
	 * 按百分比返回钳制在 0~60 之间的闪避几率。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 闪避几率：2 + 敏捷 × 0.08
	// 闪避成长最差，不靠敏捷生存。
	/** 常数项 2%：保底闪避，够触发偶尔一次的免伤，但远不够当生存手段 */
	static constexpr float BaseDodgeChance = 2.f;

	/** 敏捷 → 闪避：每 1 点敏捷 0.08%，低于游侠的 0.18，是四职业最差成长 */
	static constexpr float AgilityCoefficient = 0.08f;
};
