// Study Game Ability System _Hades

/**
 * @file MMC_Ranger_HitChance.cpp
 * @brief 游侠命中几率的 Magnitude 计算：88 + 敏捷 × 0.18 + 等级 × 0.2，钳制到 0 ~ 100。
 *
 * 要点：
 *  - 由 GE_DerivedAttributes_Ranger（Infinite，Modifier Op = Add）以 Custom Calculation Class 引用；
 *  - 基础值 88 是全职业最高档，敏捷与等级继续往上推，最终封顶 100%；
 *  - 命中判定本身（与目标闪避的对抗）不在这里做，本 MMC 只产出己方命中率数值。
 */

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_HitChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 登记命中公式的主属性输入：敏捷。
 * 派生 GE 施加给自身，所以捕获源为 Target；非快照保证敏捷变化后命中实时刷新。
 */
UMMC_Ranger_HitChance::UMMC_Ranger_HitChance()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetAgilityAttribute());
}

/**
 * 计算游侠命中几率，被 GAS 在 GE 求值时调用（敏捷变化或等级变化会再次触发）。
 * 输入：敏捷（目标捕获）、角色等级（从 Spec 的来源角色解析）。
 * 公式：命中率 = 88 + 敏捷 × 0.18 + 等级 × 0.2
 *  - 基础 88%：高起手命中，游侠的「天生准」由它定义；
 *  - 敏捷项 0.18：与闪避同系数，堆敏捷同时获得「打得中」与「躲得开」；
 *  - 等级项 0.2：等级带来的稳步命中成长，后期几乎必然到 100%。
 * 系数定义在头文件的 static constexpr 中，改数值不要动这里的算式。
 */
float UMMC_Ranger_HitChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 标签过滤参数：供属性聚合器按来源/目标标签筛选加成
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	// 未登记或读取失败返回 0，且保证非负
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);

	// 等级来自 Spec 的来源角色；解析失败返回 0，命中仍由基础值 + 敏捷项保证高水准
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHitChance + Agility * AgilityCoefficient + Level * LevelCoefficient;

	// 上限钳制：最高 100。命中率超过 100 没有任何判定意义；
	// 下限 0 防御异常负值（负命中会让必定打空，不是设计意图）。
	return FMath::Clamp(Result, 0.f, 100.f);
}
