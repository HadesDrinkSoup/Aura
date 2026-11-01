// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MaxHealth.generated.h"

/**
 * 元素法师 · 最大生命值
 * 公式：65 + 活力 × 7 + 力量 × 2 + 等级 × 7（钳制：最低 1）
 * 体魄最弱：血量主要来自等级，活力收益明显低于战士。
 *
 * 单位：生命点数。最低 1 的钳制不是随手写的：MaxHealth 为 0 会让 Health 被钳成 0、
 * 并让所有 "Health / MaxHealth" 的比例计算（血条、受击表现）出现除零，
 * 因此这里保底 1 点，保证最差情况下角色是"活着但极脆"，而不是数值崩溃。
 *
 * 与本属性相关的连锁行为：本类的结果写入 UAuraAttributeSet::MaxHealth 时会命中
 * PostAttributeChange，由它立刻把当前 Health 重新钳到新上限——
 * 也就是说 MaxHealth 变小（例如卸下装备、Debuff）会立即压低血条。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MaxHealth : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：登记公式用到的两个主属性（活力、力量）；等级从 EffectContext 解析 */
	UMMC_Elementalist_MaxHealth();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取活力、力量与角色等级，
	 * 返回不小于 1 的最大生命值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 最大生命值：65 + 活力 × 7 + 力量 × 2 + 等级 × 7
	// 体魄最弱：血量主要来自等级，活力收益明显低于战士。
	/** 常数项 65 点：1 级裸装（主属性全 0）时血量即 65 + 等级项 7 = 72，保证出生就有血 */
	static constexpr float BaseHealth = 65.f;

	/** 活力 → 生命：每 1 点活力 7 点生命，法师的主要血量来源，但只有战士（15）的一半不到 */
	static constexpr float VigorCoefficient = 7.f;

	/** 力量 → 生命：每 1 点力量 2 点生命，法师不堆力量，这一项贡献很小 */
	static constexpr float StrengthCoefficient = 2.f;

	/** 等级 → 生命：每级 7 点，50 级贡献 350 点——法师的血量主要靠等级而不是属性撑起来 */
	static constexpr float LevelCoefficient = 7.f;
};
