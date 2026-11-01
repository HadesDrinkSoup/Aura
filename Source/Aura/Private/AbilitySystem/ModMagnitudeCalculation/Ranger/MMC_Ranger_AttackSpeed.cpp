// Study Game Ability System _Hades

/**
 * @file MMC_Ranger_AttackSpeed.cpp
 * @brief 游侠攻速的 Magnitude 计算：110 + 敏捷 × 0.45 + 等级 × 0.2，钳制到 50 ~ 250。
 *
 * 要点：
 *  - 由 GE_DerivedAttributes_Ranger（Infinite，Modifier Op = Add）以 Custom Calculation Class 引用，
 *    返回攻速的目标绝对值；
 *  - 只捕获敏捷一个属性，等级单独从 GE 上下文解析，缺失时按 0（只影响等级项）；
 *  - 区间钳制发生在 MMC 内部，因此主属性怎么堆，写回属性的攻速都不会越过 50/250。
 */

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_AttackSpeed.h"
#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 登记攻速的输入属性：敏捷。
 * 捕获源必须是 Target——派生属性 GE 是「施加给自身」的，捕获 Source 会取不到值；
 * 非快照是攻速能随敏捷实时刷新（而不是只在施加瞬间算一次）的关键。
 */
UMMC_Ranger_AttackSpeed::UMMC_Ranger_AttackSpeed()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

/**
 * 计算游侠攻速，被 GAS 在 GE 求值时调用（敏捷变化会再次触发）。
 * 输入：敏捷（从目标捕获）、角色等级（从 GE 上下文解析）。
 * 公式：攻速 = 110 + 敏捷 × 0.45 + 等级 × 0.2
 *  - 基础 110：四职业中最高的起点，回应「攻速最高」的职业定位；
 *  - 敏捷项 0.45：全职业最高系数，游侠输出几乎全部押在敏捷上；
 *  - 等级项 0.2：随等级缓慢抬升，避免低级时攻速过慢、高级时不靠装备也过快。
 * 系数写在头文件的 static constexpr 中，改数值不要动这里的算式。
 */
float UMMC_Ranger_AttackSpeed::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 标签过滤参数：让属性聚合器知道本次求值的来源/目标标签
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	// 未登记或读取失败返回 0，且保证非负
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	// 等级解析失败返回 0，攻速只由基础值与敏捷项决定
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseAttackSpeed + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 区间钳制：50 ~ 250。下限保证再弱的角色也能打出普攻，
	// 上限是策划给定的天花板，避免敏捷/等级叠加后攻速线性增长失控。
	return FMath::Clamp(Result, 50.f, 250.f);
}
