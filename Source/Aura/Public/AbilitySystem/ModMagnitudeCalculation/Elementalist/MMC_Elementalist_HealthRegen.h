// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_HealthRegen.generated.h"

/**
 * 元素法师 · 生命恢复
 * 公式：0.5 + 活力 × 0.08 + 等级 × 0.03（钳制：最低 0）
 * 基础回血最低，续航依赖法力与技能。
 *
 * 单位：每秒恢复的生命值（HP/s），与 AuraGamepalyTags.h 中
 * Attributes.Derived.HealthRegen 的语义（"每秒自动恢复的生命值"）一致。
 *
 * 为什么这样设计：0.5 基础值、0.08/点活力、0.03/级都是四职业最低。
 * 法师的续航来自"法力循环 + 技能吸血/护盾"这套设计，而不是自然回血，
 * 所以脱战回血对法师几乎不构成续航手段，只是一种非常缓慢的恢复兜底。
 */
UCLASS()
class AURA_API UMMC_Elementalist_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：只登记活力一个属性捕获；等级在计算时从 EffectContext 解析 */
	UMMC_Elementalist_HealthRegen();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取活力与角色等级，
	 * 返回以 HP/s 为单位、不小于 0 的生命恢复值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 生命恢复：0.5 + 活力 × 0.08 + 等级 × 0.03
	// 基础回血最低，续航依赖法力与技能。
	/** 常数项 0.5 HP/s：四职业最低的基础回血，勉强算"站着会缓慢回一点" */
	static constexpr float BaseHealthRegen = 0.5f;

	/** 活力 → 回血：每 1 点活力 0.08 HP/s，收益只有战士的一半（0.18） */
	static constexpr float VigorCoefficient = 0.08f;

	/** 等级 → 回血：每级 0.03 HP/s，50 级也只有 1.5 HP/s，升级几乎不改善续航 */
	static constexpr float LevelCoefficient = 0.03f;
};
