// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_HealthRegen.generated.h"

/**
 * 坦克 · 生命恢复
 * 公式：1 + 活力 × 0.12 + 等级 × 0.04（钳制：最低 0）
 * 回血次高：长时间站场，脱战后恢复快。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「生命恢复」这一个 Modifier 以 Custom Calculation Class 引用。
 * 单位是「生命点」——每次结算周期回多少血由消费方决定（本仓库 C++ 目前没有回血 tick，
 * 数值只进属性面板），MMC 只负责给出这个周期量。
 * 不设上限：回血是线性续航项，硬顶会让高活力配装失去意义，只需保证不为负。
 */
UCLASS()
class AURA_API UMMC_Tank_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：只登记活力。
	 * 回血只跟体魄走，与等级项共同构成续航；捕获为非快照，活力一旦变化（升级、装备）就重算。
	 */
	UMMC_Tank_HealthRegen();

	/**
	 * 计算生命恢复幅度：BaseHealthRegen + 活力 × 0.12 + 角色等级 × 0.04。
	 * 单点活力收益低于战士（0.18），但坦克活力总量是四职业最高（主属性 46%），
	 * 按 CurveTable 推算 L1 ≈ 3.08、L50 ≈ 13.2，最终仍排第二，仅次于战士的 14.76。
	 * @param Spec 正在结算的 GE Spec
	 * @return 生命恢复量（生命点），保底 0
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 生命恢复：1 + 活力 × 0.12 + 等级 × 0.04
	// 回血次高：长时间站场，脱战后恢复快。
	/** 基础生命恢复（生命点）：1，保证活力为 0 时也有最基础的脱战回复 */
	static constexpr float BaseHealthRegen = 1.f;

	/** 每 1 点活力换取的生命恢复（生命点/点）：0.12，低于战士 0.18，靠活力总量弥补 */
	static constexpr float VigorCoefficient = 0.12f;

	/** 每 1 角色等级附加的生命恢复（生命点/级）：0.04，满级约 +2，与游侠同级 */
	static constexpr float LevelCoefficient = 0.04f;
};
