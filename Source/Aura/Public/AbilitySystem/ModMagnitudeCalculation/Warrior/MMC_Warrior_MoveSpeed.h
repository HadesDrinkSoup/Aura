// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MoveSpeed.generated.h"

/**
 * 战士 · 移动速度
 * 公式：390 + 敏捷 × 0.3 + 力量 × 0.05（钳制：最低 0）
 * 移速中等（L50 约 405，游侠约 458.5 最快）：敏捷是主权重，力量提供极少额外移速。
 *
 * 在系统里的角色：派生属性 MoveSpeed 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 MoveSpeed 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 本公式没有等级项：移速完全由属性决定，升级不会让角色跑得更快，
 *    避免"等级膨胀把战场节奏拖快"以及早后期手感差异过大；
 *  - 系数只有 0.3 / 0.05，敏捷从个位数堆到 39 也只多约 10 点，
 *    说明移速不是战士的 build 方向——真正的移速差异应来自 Buff/装备的 Add；
 *  - 两个捕获（敏捷 + 力量）都登记，任一变化都会触发重算。
 *
 * 量纲：UE 的速度单位（单位/秒，基准 390）；角色移动组件按此值驱动位移。
 *
 * 调参：数值全部来自下面 3 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_MoveSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（敏捷 + 力量，目标侧、非快照） */
	UMMC_Warrior_MoveSpeed();

	/**
	 * 计算移动速度 = 390 + 敏捷 × 0.3 + 力量 × 0.05，结果不低于 0。
	 * 由 GAS 在 GE 施加时及敏捷/力量变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取敏捷与力量
	 * @return 移动速度（>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 移动速度：390 + 敏捷 × 0.3 + 力量 × 0.05
	// 移速中等，力量提供极少额外移速。
	/** 基础移动速度：390 的白板基准，决定战士的整体机动性定位（低于游侠同档基准） */
	static constexpr float BaseMoveSpeed = 390.f;

	/** 敏捷系数：每 1 点敏捷 +0.3 移速，是移速的主权重，但绝对增量仍然很小 */
	static constexpr float AgilityCoefficient = 0.3f;

	/** 力量系数：每 1 点力量 +0.05 移速，象征性的重量级补偿，几乎不影响手感 */
	static constexpr float StrengthCoefficient = 0.05f;
};
