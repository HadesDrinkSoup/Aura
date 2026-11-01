// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_CriticalChance.generated.h"

/**
 * 坦克 · 暴击几率
 * 公式：3 + 力量 × 0.04 + 敏捷 × 0.03（钳制：0 ~ 75）
 * 暴击最低：慢速重击，不追求爆发。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「暴击几率」这一个 Modifier 以 Custom Calculation Class 引用。
 * 公式里刻意没有等级项：等级已经通过主属性 CurveTable 间接进入，
 * 再乘一次等级会让「高等级 = 高暴击」双重叠加，坦克会失去「暴击最低」的职业定位。
 * 钳制 0 ~ 75 只约束本 MMC 的输出；其它 GE 的加法叠加发生在属性聚合器层面、不经过这里，
 * 若将来要通过 Buff 继续加暴击且仍需封顶，得把钳制下沉到 UAuraAttributeSet::PreAttributeChange。
 */
UCLASS()
class AURA_API UMMC_Tank_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：登记力量（主）与敏捷（副）两个基础属性。
	 * 暴击是坦克唯一吃力量的几率类属性；两者均为非快照捕获，主属性变化即重算。
	 */
	UMMC_Tank_CriticalChance();

	/**
	 * 计算暴击几率幅度：BaseCriticalChance + 力量 × 0.04 + 敏捷 × 0.03。
	 * 按 CurveTable 主属性推算，L1 ≈ 3.58、L50 ≈ 5.79，是四职业最低且几乎没有成长感——
	 * 这是有意的：坦克靠站桩承伤，不靠爆发。
	 * @param Spec 正在结算的 GE Spec（本公式不取等级）
	 * @return 暴击几率（百分点），钳制在 0 ~ 75
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 暴击几率：3 + 力量 × 0.04 + 敏捷 × 0.03
	// 暴击最低：慢速重击，不追求爆发。
	/** 基础暴击（百分点）：3，低于元素 4 / 战士 5 / 游侠 8 */
	static constexpr float BaseCriticalChance = 3.f;

	/** 每 1 点力量换取的暴击（百分点/点）：0.04，主加成；坦克力量投点 26%，是暴击的主要来源 */
	static constexpr float StrengthCoefficient = 0.04f;

	/** 每 1 点敏捷换取的暴击（百分点/点）：0.03，副加成；坦克敏捷只有 16% 投点，贡献有限 */
	static constexpr float AgilityCoefficient = 0.03f;
};
