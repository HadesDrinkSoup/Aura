// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_CooldownReduction.generated.h"

/**
 * 战士 · 冷却缩减
 * 公式：0 + 智力 × 0.08 + 等级 × 0.03（钳制：0 ~ 40）
 * 冷却缩减较低：智力是次要属性、系数又低，技能循环明显偏慢（坦克最低，战士倒数第二）。
 *
 * 在系统里的角色：派生属性 CooldownReduction 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 CooldownReduction 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 基础项为 0 是本职业的刻意设定：战士没有"白送"的冷却缩减，想减 CD 只能堆智力或升级，
 *    因此力量/活力流的战士 CD 会一直贴在个位数；
 *  - 单位为百分比刻度（0 ~ 40 即 0% ~ 40% 的冷却缩短），上限由本类内部钳制，
 *    这样后续 Buff 用 Add 叠加也不会把技能 CD 压到 0 或负数。
 *
 * 调参：数值全部来自下面 3 个 static constexpr（钳制上下限写死在 .cpp 里），改完需重新编译。
 */
UCLASS()
class AURA_API UMMC_Warrior_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（仅智力，目标侧、非快照） */
	UMMC_Warrior_CooldownReduction();

	/**
	 * 计算冷却缩减 = 0 + 智力 × 0.08 + 等级 × 0.03，结果钳在 [0, 40]。
	 * 由 GAS 在 GE 施加时及智力/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取智力与解析角色等级
	 * @return 冷却缩减百分比（0 ~ 40）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 冷却缩减：0 + 智力 × 0.08 + 等级 × 0.03
	// 冷却缩减较低：智力是次要属性、系数又低，技能循环明显偏慢。
	/** 基础冷却缩减：写成 0.f 而不是省掉这一项，是为了让公式形状与其它职业一致、便于横向对照调参 */
	static constexpr float BaseCooldownReduction = 0.f;

	/** 智力系数：每 1 点智力 +0.08%（L50 智力 22 仅 +1.76%），战士几乎不可能靠智力拿到可观 CD */
	static constexpr float IntelligenceCoefficient = 0.08f;

	/** 等级系数：每级 +0.03%（L50 = +1.5%），是本职业冷却缩减的主要来源 */
	static constexpr float LevelCoefficient = 0.03f;
};
