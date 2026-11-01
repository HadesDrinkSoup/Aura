// Study Game Ability System _Hades

/**
 * @file MMC_Ranger_Armor.cpp
 * @brief 游侠护甲的 Magnitude 计算：1 + 活力 × 1 + 力量 × 0.35 + 等级 × 0.25，结果钳到非负。
 *
 * 要点：
 *  - 由 GE_DerivedAttributes_Ranger（Infinite，Modifier Op = Add）按 Custom Calculation Class 引用，
 *    返回的是护甲的目标绝对值而不是增量；
 *  - 捕获非快照，活力/力量变化时 GAS 自动重算，因此公式只需描述「当前应有值」；
 *  - 属性读取失败、捕获未登记、等级解析失败都退化为 0，异常只会让护甲偏低，不会中断 GE 求值。
 */

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_Armor.h"
#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 登记护甲公式用到的两个主属性：活力、力量。
 * 派生属性 GE 是施加给自身的，所以捕获源固定为 Target；
 * 只有登记过的属性才参与聚合器求值，也才能把「主属性变化 → 护甲重算」这条监听建起来。
 */
UMMC_Ranger_Armor::UMMC_Ranger_Armor()
{
	CaptureTargetAttributes(UAuraAttributeSet::GetVigorAttribute(),
		UAuraAttributeSet::GetStrengthAttribute());
}

/**
 * 计算游侠护甲，被 GAS 在 GE 求值时调用（非快照捕获的属性变化会再次触发）。
 * 输入：活力、力量（均从目标捕获）、角色等级（从 GE 上下文解析，SourceObject 优先、Instigator 兜底）。
 * 公式：护甲 = 1 + 活力 × 1 + 力量 × 0.35 + 等级 × 0.25
 *  - 基础 1 点：保证裸装/0 级时也有护甲，不依赖任何属性；
 *  - 活力项是主收益（每点 1 点护甲），力量项只有 0.35 的零头，体现游侠不靠力量生存；
 *  - 等级项让护甲随等级线性成长，抵消游侠活力偏低的问题。
 * 系数全部定义在头文件的 static constexpr 里，改数值不要动这里的算式。
 */
float UMMC_Ranger_Armor::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 带上来源/目标标签，供属性聚合器按标签过滤加成（与捕获登记一起决定最终取值）
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	// 两个输入都走 GetCapturedMagnitude：未登记或读取失败按 0，且保证取到的是非负值
	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);
	const float Strength = GetCapturedMagnitude(
		UAuraAttributeSet::GetStrengthAttribute(), Spec, EvaluationParameters);
	// 等级：先 SourceObject 再 Instigator；都解析不到返回 0，此时护甲只由属性项构成
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseArmor + Vigor * VigorCoefficient + Strength * StrengthCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0。MMC 的输出会被 GE 以 Add 方式写进 Armor，
	// 负护甲对后续减伤与 UI 展示都没有意义；正常输入不会为负，这里只是防御异常数据。
	return FMath::Max(Result, 0.f);
}
