// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_Armor.generated.h"

/**
 * 元素法师 · 护甲
 * 公式：0 + 活力 × 0.8 + 力量 × 0.2 + 等级 × 0.15（钳制：最低 0）
 * 基础护甲为 0，只能获得极低的物理减免。
 *
 * 在 GAS 中的位置：由派生属性 GE（GE_DerivedAttributes_Elementalist）的一个 Modifier 以
 * 「Custom Calculation Class」引用，Duration Policy 为 Infinite，捕获源为 Target；
 * 活力/力量/等级任一变化时聚合器会重走本类，结果覆盖写入 UAuraAttributeSet::Armor
 * （该属性带 REPNOTIFY_OnChanged 复制，客户端由 OnRep_Armor 接收）。
 *
 * 为什么这样设计：护甲是元素法师最弱的一项防御——没有常数项、力量收益只有 0.2，
 * 面板护甲基本是"活力和等级顺手带出来的副产品"，堆护甲不是这个职业的可行路线。
 */
UCLASS()
class AURA_API UMMC_Elementalist_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造函数：只登记本公式依赖的捕获属性（活力、力量），不做任何计算。
	 * 捕获方式由基类固定为「目标 + 非快照」，原因见 UAuraMMCBase::CaptureTargetAttribute。
	 */
	UMMC_Elementalist_Armor();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：每次需要护甲数值时
	 * （GE 施加瞬间 + 捕获属性/等级变化后）执行一次，返回面板护甲值，结果不小于 0。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 护甲：0 + 活力 × 0.8 + 力量 × 0.2 + 等级 × 0.15
	// 基础护甲为 0，只能获得极低的物理减免。
	/** 常数项 0 点：护甲公式没有基础值，1 级裸装（主属性全 0）时面板只剩等级项带来的 0.15 */
	static constexpr float BaseArmor = 0.f;

	/** 活力 → 护甲：每 1 点活力 0.8 点护甲；这是法师护甲最主要的来源 */
	static constexpr float VigorCoefficient = 0.8f;

	/** 力量 → 护甲：每 1 点力量 0.2 点护甲；法师不堆力量，这一项基本可以忽略 */
	static constexpr float StrengthCoefficient = 0.2f;

	/** 等级 → 护甲：每级 0.15 点护甲，50 级也只有 7.5 点，升级带来的物理减伤很少 */
	static constexpr float LevelCoefficient = 0.15f;
};
