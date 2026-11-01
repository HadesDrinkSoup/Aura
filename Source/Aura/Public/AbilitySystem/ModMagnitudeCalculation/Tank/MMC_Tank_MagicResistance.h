// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_MagicResistance.generated.h"

/**
 * 坦克 · 魔法抗性
 * 公式：4 + 活力 × 1.4 + 智力 × 0.3 + 等级 × 1.4（钳制：最低 0）
 * 魔抗最高：活力主导、智力补足，生命 / 护甲 / 魔抗三项全部第一，满级约 200。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「魔法抗性」这一个 Modifier 以 Custom Calculation Class 引用。
 * 与护甲一样是「点数」而非百分比：下游按「抗性 / (抗性 + 常数)」换算减伤，所以不设上限。
 * 活力系数与等级系数同为 1.4，是因为坦克智力投点只有 12%——魔抗必须靠等级项补足，
 * 否则「坦克魔抗第一」的定位会在高等级被智力型职业反超。
 */
UCLASS()
class AURA_API UMMC_Tank_MagicResistance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：登记活力（主收益）与智力（补充收益）。
	 * 智力只用来让「坦克顺手堆到的智力」不至于浪费，实际权重很低；
	 * 两个属性都是非快照捕获，活力/智力变化会触发魔抗重算。
	 */
	UMMC_Tank_MagicResistance();

	/**
	 * 计算魔法抗性幅度：BaseMagicResistance + 活力 × 1.4 + 智力 × 0.3 + 角色等级 × 1.4。
	 * 按 CurveTable 主属性推算，L1 ≈ 30.4、L50 ≈ 199.6（战士 ≈ 150.5，第二名），
	 * 其中满级等级项贡献约 70 点、约占三分之一，是坦克维持魔抗领先的关键。
	 * @param Spec 正在结算的 GE Spec
	 * @return 魔法抗性点数，保底 0
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 魔法抗性：4 + 活力 × 1.4 + 智力 × 0.3 + 等级 × 1.4
	// 魔抗最高：活力主导、智力补足，生命 / 护甲 / 魔抗三项全部第一；满级约 200。
	/** 基础魔抗（点）：4，四职业最高（元素 3 / 游侠 2 / 战士 1） */
	static constexpr float BaseMagicResistance = 4.f;

	/** 每 1 点活力换取的魔抗（点/点）：1.4，主收益；坦克活力投点 46%，是魔抗的绝对主力 */
	static constexpr float VigorCoefficient = 1.4f;

	/** 每 1 点智力换取的魔抗（点/点）：0.3，副收益；坦克智力仅 12% 投点，占比很小 */
	static constexpr float IntelligenceCoefficient = 0.3f;

	/** 每 1 角色等级附加的魔抗（点/级）：1.4，与活力系数持平；是护甲等级项（0.7）的两倍 */
	static constexpr float LevelCoefficient = 1.4f;
};
