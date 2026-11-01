// Study Game Ability System _Hades

/**
 * @file MMC_Ranger_DodgeChance.cpp
 * @brief 游侠闪避几率的 Magnitude 计算：6 + 敏捷 × 0.18，钳制到 0 ~ 60。
 *
 * 要点：
 *  - 由 GE_DerivedAttributes_Ranger（Infinite，Modifier Op = Add）以 Custom Calculation Class 引用；
 *  - 全文只捕获敏捷一个属性，也不解析角色等级，是游侠公式中最简单的一条；
 *  - 上限 60% 在 MMC 内部钳制：闪避越高，继续堆敏捷的边际收益越低，封顶避免「几乎打不中」。
 */

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_DodgeChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 登记闪避唯一的输入属性：敏捷。
 * 派生属性 GE 施加给自身，捕获源必须是 Target；非快照是闪避随敏捷实时刷新的前提。
 */
UMMC_Ranger_DodgeChance::UMMC_Ranger_DodgeChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

/**
 * 计算游侠闪避几率，被 GAS 在 GE 求值时调用（敏捷变化会再次触发）。
 * 输入：敏捷（从目标捕获）。
 * 公式：闪避率 = 6 + 敏捷 × 0.18
 *  - 基础 6%：起手闪避，使游侠在低等级就有「靠走位与运气活下来」的手感；
 *  - 敏捷项 0.18：全职业最高，闪避是敏捷在防御端的唯一出口（游侠护甲/魔抗都很低）。
 * 系数定义在头文件的 static constexpr 中，改数值不要动这里的算式。
 */
float UMMC_Ranger_DodgeChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 标签过滤参数：供属性聚合器按来源/目标标签筛选加成
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	// 未登记或读取失败返回 0，且保证非负
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	const float Result = BaseDodgeChance + Agility * AgilityCoefficient;

	// 上限钳制：最高 60。闪避是概率防御，封顶防止高敏捷角色几乎不吃任何攻击命中；
	// 下限 0 只用于兜住异常输入（绝不该出现负闪避）。
	return FMath::Clamp(Result, 0.f, 60.f);
}
