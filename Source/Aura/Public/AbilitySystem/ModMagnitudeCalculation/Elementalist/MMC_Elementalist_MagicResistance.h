// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MagicResistance.generated.h"

/**
 * 元素法师 · 魔法抗性
 * 公式：3 + 活力 × 0.45 + 智力 × 0.35 + 等级 × 0.4（钳制：最低 0）
 * 魔抗不再是强项：智力收益压到 0.35、活力 0.45，只比游侠高一点，被法术打到同样痛。
 *
 * 单位：减伤点数，与 Armor 同量纲，只是一个挡物理、一个挡法术。
 *
 * 为什么这样设计：故意不让"智力"直接等价于"魔抗"——如果法系主属性换成高魔抗，
 * 法师会同时拥有高输出与高法术抗性，对抗法术就没有弱点；0.35 的系数保证了
 * 法师在法术面前依然脆弱，抗性需要靠装备/护盾补齐。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：登记公式用到的两个主属性（活力、智力）；等级从 EffectContext 解析 */
	UMMC_Elementalist_MagicResistance();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取活力、智力与角色等级，
	 * 返回不小于 0 的魔法抗性面板值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 魔法抗性：3 + 活力 × 0.45 + 智力 × 0.35 + 等级 × 0.4
	// 魔抗不再是强项：智力收益压到 0.35、活力 0.45，只比游侠高一点，被法术打到同样痛。
	/** 常数项 3 点：比游侠（2）略高的保底魔抗，但也就"聊胜于无"的水平 */
	static constexpr float BaseMagicResistance = 3.f;

	/** 活力 → 魔抗：每 1 点活力 0.45 点，是法师魔抗的主要来源（仍远低于坦克/战士） */
	static constexpr float VigorCoefficient = 0.45f;

	/** 智力 → 魔抗：每 1 点智力 0.35 点；刻意压低，避免"法强高=魔抗高"的滚雪球 */
	static constexpr float IntelligenceCoefficient = 0.35f;

	/** 等级 → 魔抗：每级 0.4 点，50 级约 20 点，抗性的增长基本只来自等级 */
	static constexpr float LevelCoefficient = 0.4f;
};
