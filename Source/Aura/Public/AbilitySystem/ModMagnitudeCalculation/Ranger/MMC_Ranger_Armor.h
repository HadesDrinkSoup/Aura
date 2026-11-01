// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_Armor.generated.h"

/**
 * 游侠 · 护甲
 * 公式：1 + 活力 × 1 + 力量 × 0.35 + 等级 × 0.25（钳制：最低 0）
 * 护甲偏低，被近身时风险很高。
 *
 * 在系统中的位置：
 *  - 作为 GE_DerivedAttributes_Ranger 中 Armor 这个 Modifier 的 Calculation Class 被引用，
 *    该 GE 由 AAuraCharacterBase::InitDefaultAttributes 在服务器上施加给角色自身；
 *  - GE 为 Infinite 且下面的捕获是非快照的，活力/力量一旦变化 GAS 就会重新调用本类并写回 Armor，
 *    所以这里算的是「当前应有的护甲总量」，而不是一次性的增量；
 *  - Modifier Op 为 Add，本函数返回的数值会直接作为 Armor 的幅度写进去。
 *
 * 量纲：护甲点数（无量纲点值，不是百分比），数值越大物理减伤越强。
 * 调参：只改下面的 static constexpr 常量，不要改 cpp 里的算式，否则公式与本文档注释会脱节。
 */
UCLASS()
class AURA_API UMMC_Ranger_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/**
	 * 登记护甲公式依赖的两个输入属性：活力、力量。
	 * 二者都以「目标 + 非快照」方式捕获，这是 Infinite 派生 GE 能随主属性实时重算的前提；
	 * 漏登记不会编译报错，只会在求值时告警并退化成 0（护甲静默变小），所以改动公式时务必同步这里。
	 */
	UMMC_Ranger_Armor();

	/**
	 * 计算游侠护甲的目标幅度：1 + 活力 × 1 + 力量 × 0.35 + 等级 × 0.25，结果不低于 0。
	 * 每次 GE 求值（含主属性变化触发的重算）都会被调用；等级解析不到时按 0 处理，等级项自然为 0。
	 * @param Spec 触发本次求值的 GE Spec，属性与等级都从它携带的上下文里解析。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 护甲：1 + 活力 × 1 + 力量 × 0.35 + 等级 × 0.25
	// 护甲偏低，被近身时风险很高。
	/** 基础护甲（点）：与任何属性都无关的保底常量，配合末尾的钳制保证护甲不会出现负值 */
	static constexpr float BaseArmor = 1.f;

	/** 每 1 点活力贡献的护甲点数；游侠的活力收益明显低于战士（1.8）与坦克（2.6） */
	static constexpr float VigorCoefficient = 1.f;
	/** 每 1 点力量贡献的护甲点数；力量是游侠的副属性，仅为活力收益的约三分之一 */
	static constexpr float StrengthCoefficient = 0.35f;
	/** 每 1 个角色等级贡献的护甲点数；让护甲随等级线性成长，避免低活力时护甲长期贴着 0 */
	static constexpr float LevelCoefficient = 0.25f;
};
