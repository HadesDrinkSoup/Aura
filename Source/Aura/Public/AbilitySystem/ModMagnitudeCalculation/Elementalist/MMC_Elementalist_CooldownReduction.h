// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_CooldownReduction.generated.h"

/**
 * 元素法师 · 冷却缩减
 * 公式：3 + 智力 × 0.25 + 等级 × 0.1（钳制：0 ~ 40）
 * 冷却缩减核心职业，智力收益最高。
 *
 * 单位：百分比。40 表示"技能冷却缩短 40%"，不是 40 秒；
 * 上下限都参与玩法平衡，见 cpp 中对 Clamp 的说明。
 *
 * 为什么这样设计：智力 0.25/点是四职业最高，配合 3 点基础值让它成为"冷却缩减核心职业"；
 * 上限压到 40 既给后续 Buff/装备留出空间，又不会出现技能无 CD 的刷屏循环。
 */
UCLASS()
class AURA_API UMMC_Elementalist_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：登记本公式唯一的属性依赖（智力）；等级在计算时从 EffectContext 解析 */
	UMMC_Elementalist_CooldownReduction();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取智力与角色等级，
	 * 返回按百分比钳制在 0~40 的冷却缩减值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 冷却缩减：3 + 智力 × 0.25 + 等级 × 0.1
	// 冷却缩减核心职业，智力收益最高。
	/** 常数项 3%：不投属性也有 3% 冷却缩减，保证起手就有一点技能循环优势 */
	static constexpr float BaseCooldownReduction = 3.f;

	/** 智力 → 冷却缩减：每 1 点智力 0.25%，是四职业里最高的智力收益 */
	static constexpr float IntelligenceCoefficient = 0.25f;

	/** 等级 → 冷却缩减：每级 0.1%，50 级约 5%，只是缓慢的自然成长 */
	static constexpr float LevelCoefficient = 0.1f;
};
