// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_AttackSpeed.generated.h"

/**
 * 坦克 · 攻击速度
 * 公式：90 + 敏捷 × 0.15 + 等级 × 0.1（钳制：50 ~ 250）
 * 攻速最低：重甲盾牌，出手慢但单次结实。
 *
 * 在系统中的角色：只被 GE_DerivedAttributes_Tank 里「攻击速度」这一个 Modifier 以 Custom Calculation Class 引用。
 * 它是无量纲的相对攻速值（设计基准 100 附近）；本仓库 C++ 中没有消费方，
 * 如何映射到攻击间隔/动画播放速率由蓝图侧读取该属性后决定。
 * 钳制 50 ~ 250 的原因：攻速会被当作乘性系数使用，降到 0 或负数会让攻击间隔发散，
 * 上限则防止等级成长叠加后把攻速推到离谱的数值。
 */
UCLASS()
class AURA_API UMMC_Tank_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 构造：只登记敏捷。
	 * 坦克攻速不吃活力/力量，而敏捷只占坦克主属性预算的 16%，这就是它攻速四职业垫底的根因；
	 * 非快照捕获保证敏捷变化（升级、Buff）后攻速会被 GAS 自动重算。
	 */
	UMMC_Tank_AttackSpeed();

	/**
	 * 计算攻速幅度：BaseAttackSpeed + 敏捷 × 0.15 + 角色等级 × 0.1。
	 * 敏捷系数与元素法师并列最低（游侠 0.45），等级项则让低敏捷成长的坦克不至于一直停在 90 附近。
	 * @param Spec 正在结算的 GE Spec
	 * @return 攻速值，钳制在 50 ~ 250
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 攻击速度：90 + 敏捷 × 0.15 + 等级 × 0.1
	// 攻速最低：重甲盾牌，出手慢但单次结实。
	/** 基础攻速（无量纲，设计基准 100）：四职业最低（元素 95 / 战士 100 / 游侠 110） */
	static constexpr float BaseAttackSpeed = 90.f;

	/** 每 1 点敏捷换取的攻速（攻速值/点）：0.15，与元素法师并列最低 */
	static constexpr float AgilityCoefficient = 0.15f;

	/** 每 1 角色等级附加的攻速（攻速值/级）：0.1，满级约 +5，与元素/战士持平（游侠 0.2） */
	static constexpr float LevelCoefficient = 0.1f;
};
