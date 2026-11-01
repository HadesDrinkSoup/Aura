// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_AttackSpeed.generated.h"

/**
 * 战士 · 攻击速度
 * 公式：100 + 敏捷 × 0.2 + 等级 × 0.1（钳制：50 ~ 250）
 * 攻速次高：敏捷与等级共同驱动，重视单次伤害而非攻击频率（基准刻度为 100）。
 *
 * 在系统里的角色：派生属性 AttackSpeed 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 AttackSpeed 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 只捕获敏捷：攻速不吃力量/活力，所以堆力量的战士不会因为力量提升而变快，
 *    这是有意的定位——战士的强度体现在单次伤害（PhysicalAttack 力量系数 3.2）而不是出手频率；
 *  - 数值是"相对基准的攻速刻度"，100 = 基准速度，消费方（动画播放率/技能节奏）按此比例换算。
 *
 * 调参：数值全部来自下面 3 个 static constexpr（含钳制上下限写死在 .cpp 里），改完需重新编译。
 */
UCLASS()
class AURA_API UMMC_Warrior_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（仅敏捷，目标侧、非快照） */
	UMMC_Warrior_AttackSpeed();

	/**
	 * 计算攻速 = 100 + 敏捷 × 0.2 + 等级 × 0.1，结果钳在 [50, 250]。
	 * 由 GAS 在 GE 施加时及敏捷/等级变化后调用；纯函数，不读随机数。
	 * @param Spec 正在计算的 GE Spec，用于读取敏捷与解析角色等级
	 * @return 攻速刻度（50 ~ 250，100 为基准）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 攻击速度：100 + 敏捷 × 0.2 + 等级 × 0.1
	// 攻速次高：敏捷与等级共同驱动，重视单次伤害而非攻击频率。
	/** 基准攻速刻度：等价于"1 倍速"，也是没有任何敏捷/等级时的下限参照 */
	static constexpr float BaseAttackSpeed = 100.f;

	/** 敏捷系数：每 1 点敏捷 +0.2 刻度（L50 敏捷 39 只贡献 +7.8），所以攻速成长平缓 */
	static constexpr float AgilityCoefficient = 0.2f;

	/** 等级系数：每级 +0.1 刻度（L50 = +5），保证满级后基础手感比 L1 略快 */
	static constexpr float LevelCoefficient = 0.1f;
};
