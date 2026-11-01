// Study Game Ability System _Hades

/**
 * @file MMC_Ranger_CriticalChance.cpp
 * @brief 游侠暴击几率的 Magnitude 计算：8 + 敏捷 × 0.2 + 智力 × 0.03，钳制到 0 ~ 75。
 *
 * 要点：
 *  - 由 GE_DerivedAttributes_Ranger（Infinite，Modifier Op = Add）以 Custom Calculation Class 引用；
 *  - 不读取角色等级：暴击率只随敏捷/智力成长，避免升级直接稀释「堆敏捷」的意义；
 *  - 上限 75% 在 MMC 内部钳制，超出部分由策划认为没有设计价值。
 */

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_CriticalChance.h"
#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 登记暴击公式的两个输入属性：敏捷、智力。
 * 捕获源固定为 Target（派生 GE 施加给自身），非快照保证主属性变化后暴击值立刻重算；
 * 漏登记只会让对应项退化为 0，不会报错，所以改公式时别忘同步这里。
 */
UMMC_Ranger_CriticalChance::UMMC_Ranger_CriticalChance()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetAgilityAttribute(),
		UAuraAttributeSet::GetIntelligenceAttribute());
}

/**
 * 计算游侠暴击几率，被 GAS 在 GE 求值时调用（敏捷/智力变化会再次触发）。
 * 输入：敏捷、智力（均从目标捕获）；公式不使用角色等级。
 * 公式：暴击率 = 8 + 敏捷 × 0.2 + 智力 × 0.03
 *  - 基础 8%：四职业最高起手，呼应「暴击核心职业」的定位；
 *  - 敏捷项 0.2：绝对主力，游侠堆敏捷同时提升攻速/闪避/命中，暴击是其中一环；
 *  - 智力项 0.03：极小补贴，保证副属性不是完全废属性。
 * 系数定义在头文件的 static constexpr 中，改数值不要动这里的算式。
 */
float UMMC_Ranger_CriticalChance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 标签过滤参数：供属性聚合器按来源/目标标签筛选加成
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	// 未登记或读取失败返回 0，且保证非负
	const float Agility = GetCapturedMagnitude(
		UAuraAttributeSet::GetAgilityAttribute(), Spec, EvaluationParameters);
	const float Intelligence = GetCapturedMagnitude(
		UAuraAttributeSet::GetIntelligenceAttribute(), Spec, EvaluationParameters);

	const float Result = BaseCriticalChance + Agility * AgilityCoefficient + Intelligence * IntelligenceCoefficient;

	// 上限钳制：最高 75。概率超过 75% 后继续堆敏捷的收益会趋近于无意义，
	// 也会让「暴击/不暴击」的判定失去随机性；下限 0 防御异常负值。
	return FMath::Clamp(Result, 0.f, 75.f);
}
