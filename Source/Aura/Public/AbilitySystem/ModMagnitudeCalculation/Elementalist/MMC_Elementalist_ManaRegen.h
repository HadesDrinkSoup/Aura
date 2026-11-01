// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_ManaRegen.generated.h"

/**
 * 元素法师 · 法力恢复
 * 公式：3 + 智力 × 0.3 + 等级 × 0.1（钳制：最低 0）
 * 回蓝核心职业，支撑高频技能循环。
 *
 * 单位：每秒恢复的法力值（MP/s），与 AuraGamepalyTags.h 中
 * Attributes.Derived.ManaRegen 的语义（"每秒自动恢复的法力值"）一致。
 *
 * 为什么这样设计：基础 3 MP/s + 智力 0.3/点，配合 MaxMana 的 15/点智力，
 * 让法师同时拥有"池子最大"和"回得最快"两件事，因此可以维持高频施法；
 * 上限没有额外钳制 100 之类的值，因为消耗端（技能耗蓝）本身才是平衡阀门。
 */
UCLASS()
class AURA_API UMMC_Elementalist_ManaRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：只登记智力一个属性捕获；等级在计算时从 EffectContext 解析 */
	UMMC_Elementalist_ManaRegen();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取智力与角色等级，
	 * 返回以 MP/s 为单位、不小于 0 的法力恢复值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 法力恢复：3 + 智力 × 0.3 + 等级 × 0.1
	// 回蓝核心职业，支撑高频技能循环。
	/** 常数项 3 MP/s：四职业最高的基础回蓝，保证前期也能持续放技能 */
	static constexpr float BaseManaRegen = 3.f;

	/** 智力 → 回蓝：每 1 点智力 0.3 MP/s，四职业最高的回蓝属性收益 */
	static constexpr float IntelligenceCoefficient = 0.3f;

	/** 等级 → 回蓝：每级 0.1 MP/s，50 级约 5 MP/s，随等级缓慢加厚续航 */
	static constexpr float LevelCoefficient = 0.1f;
};
