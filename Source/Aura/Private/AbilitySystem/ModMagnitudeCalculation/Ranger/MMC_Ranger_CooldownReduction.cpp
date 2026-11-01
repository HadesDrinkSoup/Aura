// Study Game Ability System _Hades

/**
 * @file MMC_Ranger_CooldownReduction.cpp
 * @brief 游侠冷却缩减的 Magnitude 计算：1 + 敏捷 × 0.1 + 智力 × 0.05，钳制到 0 ~ 40。
 *
 * 要点：
 *  - 由 GE_DerivedAttributes_Ranger（Infinite，Modifier Op = Add）以 Custom Calculation Class 引用；
 *  - 公式没有等级项，也不读取角色等级：冷却缩减完全由敏捷/智力决定，这是游侠与其它职业的差异点；
 *  - 上限 40 在 MMC 内部钳制，40 表示最多缩短 40% 冷却，超过没有设计意义。
 */

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_CooldownReduction.h"
#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 登记冷却缩减的两个输入属性：敏捷、智力。
 * 顺序与公式一致（敏捷在前），但捕获集合本身无序，重复登记会被基类忽略；
 * 非快照捕获是「主属性涨了冷却缩减立刻跟着变」的前提。
 */
UMMC_Ranger_CooldownReduction::UMMC_Ranger_CooldownReduction()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

/**
 * 计算游侠冷却缩减，被 GAS 在 GE 求值时调用（敏捷/智力变化会再次触发）。
 * 输入：敏捷、智力（均从目标捕获）；本公式刻意不使用角色等级。
 * 公式：冷却缩减 = 1 + 敏捷 × 0.1 + 智力 × 0.05
 *  - 基础 1%：起手值，使 0 级角色也有极小的冷却收益；
 *  - 敏捷项 0.1：主收益，与游侠「敏捷主导」的定位一致；
 *  - 智力项 0.05：给副属性的小额补贴，保证堆智力也不至于完全亏。
 * 系数定义在头文件的 static constexpr 中，改数值不要动这里的算式。
 */
float UMMC_Ranger_CooldownReduction::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 标签过滤参数：供属性聚合器按来源/目标标签筛选加成
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	// 未登记或读取失败返回 0，且保证非负
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCooldownReduction + Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient;

	// 上限钳制：最高 40。不钳的话高敏捷角色会把冷却压到接近 0，技能循环会失去节奏，
	// 下限 0 则是防御异常数据（例如属性被外部系统写成负值）。
	return FMath::Clamp(Result, 0.f, 40.f);
}
