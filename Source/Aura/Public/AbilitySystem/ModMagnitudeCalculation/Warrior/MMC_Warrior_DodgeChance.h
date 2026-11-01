// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_DodgeChance.generated.h"

/**
 * 战士 · 闪避几率
 * 公式：3 + 敏捷 × 0.07（钳制：0 ~ 60）
 * 闪避偏低：敏捷系数 0.07 是四职业里最低档，战士靠护甲硬抗而不是靠闪避躲（仅次于游侠，高于坦克）。
 *
 * 在系统里的角色：派生属性 DodgeChance 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 DodgeChance 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 本公式没有等级项：闪避完全不随等级成长，L50 也只有 ~5.7%，
 *    想提升闪避只能堆敏捷或吃专门的 Buff，这与"靠护甲生存"的定位一致；
 *  - 单位是百分比（3 表示 3%）。
 *
 * 调参：数值全部来自下面 2 个 static constexpr（钳制上下限写死在 .cpp 里），改完需重新编译。
 */
UCLASS()
class AURA_API UMMC_Warrior_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（仅敏捷，目标侧、非快照） */
	UMMC_Warrior_DodgeChance();

	/**
	 * 计算闪避几率 = 3 + 敏捷 × 0.07，结果钳在 [0, 60]。
	 * 由 GAS 在 GE 施加时及敏捷变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取敏捷
	 * @return 闪避几率百分比（0 ~ 60）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 闪避几率：3 + 敏捷 × 0.07
	// 闪避偏低：敏捷系数 0.07，战士靠护甲硬抗而不是靠闪避躲。
	/** 基础闪避率（%）：3% 的白板闪避，避免早期完全无法规避伤害 */
	static constexpr float BaseDodgeChance = 3.f;

	/** 敏捷系数：每 1 点敏捷 +0.07%（L50 敏捷 39 ≈ +2.73%），说明闪避不是战士的成长方向 */
	static constexpr float AgilityCoefficient = 0.07f;
};
