// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_CriticalChance.generated.h"

/**
 * 战士 · 暴击几率
 * 公式：5 + 敏捷 × 0.12 + 力量 × 0.05（钳制：0 ~ 75）
 * 暴击次高：敏捷与力量双加成，仅次于游侠（L50 约 13.0，游侠约 22.9）。
 *
 * 在系统里的角色：派生属性 CriticalChance 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 CriticalChance 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 双主属性嵌套：敏捷给 0.12、力量给 0.05，所以既堆力量的坦克型战士、也堆敏捷的爆发型战士
 *    都能拿到暴击，但两者都不极端，这也是"暴击次高"的来源；
 *  - 单位是百分比（5 表示 5%），没有等级项——暴击不随等级自然成长，只随主属性与装备走。
 *
 * 调参：数值全部来自下面 3 个 static constexpr（钳制上下限写死在 .cpp 里），改完需重新编译。
 */
UCLASS()
class AURA_API UMMC_Warrior_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（敏捷 + 力量，目标侧、非快照） */
	UMMC_Warrior_CriticalChance();

	/**
	 * 计算暴击几率 = 5 + 敏捷 × 0.12 + 力量 × 0.05，结果钳在 [0, 75]。
	 * 由 GAS 在 GE 施加时及敏捷/力量变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取敏捷与力量
	 * @return 暴击几率百分比（0 ~ 75）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 暴击几率：5 + 敏捷 × 0.12 + 力量 × 0.05
	// 暴击次高：敏捷与力量双加成，仅次于游侠（L50 约 13）。
	/** 基础暴击率（%）：所有战士起手的 5%，保证纯力量 build 也有一点爆发手感 */
	static constexpr float BaseCriticalChance = 5.f;

	/** 敏捷系数：每 1 点敏捷 +0.12%，是暴击的主权重（L50 敏捷 39 ≈ +4.68%） */
	static constexpr float AgilityCoefficient = 0.12f;

	/** 力量系数：每 1 点力量 +0.05%，让物攻主属性也能小幅转成暴击（L50 力量 66 ≈ +3.3%） */
	static constexpr float StrengthCoefficient = 0.05f;
};
