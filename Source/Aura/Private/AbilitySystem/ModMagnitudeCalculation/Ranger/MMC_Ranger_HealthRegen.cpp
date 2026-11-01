// Study Game Ability System _Hades

/**
 * @file MMC_Ranger_HealthRegen.cpp
 * @brief 游侠生命恢复的 Magnitude 计算：0.8 + 体质 × 0.1 + 等级 × 0.04，兜底不低于 0。
 *
 * 要点：
 *  - 由 GE_DerivedAttributes_Ranger（Infinite，Modifier Op = Add）以 Custom Calculation Class 引用；
 *  - 公式同时吃「体质」与「角色等级」：体质是主动投入，等级是被动托底；
 *  - 结果量纲是每秒回血值，由使用 HealthRegen 的逻辑按帧/Tick 累积，本 MMC 只产出速率。
 */

#include "AbilitySystem/ModMagnitudeCalculation/Ranger/MMC_Ranger_HealthRegen.h"
#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 登记生命恢复唯一的主属性输入：体质。
 * 捕获源为 Target、非快照，体质变化即触发 Infinite GE 重算，
 * 所以玩家堆体质后不需要重新施加 GE 就能看到回血变快。
 */
UMMC_Ranger_HealthRegen::UMMC_Ranger_HealthRegen()
{
	CaptureTargetAttribute(UAuraAttributeSet::GetVigorAttribute());
}

/**
 * 计算游侠生命恢复速率，被 GAS 在 GE 求值时调用（体质变化或等级变化会再次触发）。
 * 输入：体质（目标捕获）、角色等级（从 Spec 的来源角色解析）。
 * 公式：回血/秒 = 0.8 + 体质 × 0.1 + 等级 × 0.04
 *  - 基础 0.8：裸装游侠脱战后每秒回 0.8 点血，属于慢速自愈；
 *  - 体质项 0.1：唯一的主动成长项，堆体质直接把恢复速度拉起来；
 *  - 等级项 0.04：给不做生存投入的角色一点随等级递增的托底，避免后期完全依赖药水。
 * 系数定义在头文件的 static constexpr 中，改数值不要动这里的算式。
 */
float UMMC_Ranger_HealthRegen::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 标签过滤参数：供属性聚合器按来源/目标标签筛选加成
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

	// 未登记或读取失败返回 0，且保证非负
	const float Vigor = GetCapturedMagnitude(
		UAuraAttributeSet::GetVigorAttribute(), Spec, EvaluationParameters);

	// 等级来自 Spec 的来源角色；解析失败返回 0，只损失等级项，不影响体质项
	const int32 Level = GetCharacterLevelFromSpec(Spec);

	const float Result = BaseHealthRegen + Vigor * VigorCoefficient + Level * LevelCoefficient;

	// 兜底：结果不低于 0。回血速率一旦为负就会变成「持续掉血」，
	// 这类异常通常来自属性被外部系统写成负值，夹到 0 以退化为「不回血」而不是持续损伤。
	return FMath::Max(Result, 0.f);
}
