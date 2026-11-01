// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_Armor.generated.h"

/**
 * 坦克 · 护甲
 * 公式：8 + 活力 × 2.6 + 力量 × 1.4 + 等级 × 0.7（钳制：最低 0）
 * 护甲最高：活力与力量双加成，物理减伤的核心。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「护甲」这一个 Modifier 以 Custom Calculation Class 引用。
 * 护甲是「点数」而不是百分比，下游按「护甲 / (护甲 + 常数)」换算成物理减伤
 * （见 Docs/PrimaryAttributes_CurveTable.md 第 6 节），所以这里不设上限：
 * 数值再高也只是减伤曲线渐近 100%，不会出现物理免伤。
 */
UCLASS()
class AURA_API UMMC_Tank_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：登记公式依赖的两个基础属性——活力（主收益）与力量（副收益）。
	 * 捕获清单必须在构造期固化（GAS 在 GE 结算前就要知道该监听哪些属性），
	 * 且二者都用 Target 源 + 非快照，因此坦克自身的升级/装备/Buff 改动主属性都会触发护甲重算。
	 */
	UMMC_Tank_Armor();

	/**
	 * 计算护甲幅度：BaseArmor + 活力 × 2.6 + 力量 × 1.4 + 角色等级 × 0.7。
	 * 等级取自 EffectContext 解析出的角色等级（SourceObject → Instigator 兜底），不是 GE 自身的 Level，
	 * 因此同一套公式套在敌人身上也能按其等级给出对应护甲。
	 * @param Spec 正在结算的 GE Spec，用于取捕获属性值与来源/目标标签
	 * @return 护甲点数，保底 0
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 护甲：8 + 活力 × 2.6 + 力量 × 1.4 + 等级 × 0.7
	// 护甲最高：活力与力量双加成，物理减伤的核心。
	/** 基础护甲（点）：主属性 0、等级 0 时的裸装底值 */
	static constexpr float BaseArmor = 8.f;

	/** 每 1 点活力换取的护甲（点/点）：坦克的主护甲来源，主属性预算里 46% 投在活力上 */
	static constexpr float VigorCoefficient = 2.6f;

	/** 每 1 点力量换取的护甲（点/点）：约为活力收益的 54%，奖励副属性投点但不喧宾夺主 */
	static constexpr float StrengthCoefficient = 1.4f;

	/** 每 1 角色等级附加的护甲（点/级）：不依赖主属性的保底成长，等级由 EffectContext 解析 */
	static constexpr float LevelCoefficient = 0.7f;
};
