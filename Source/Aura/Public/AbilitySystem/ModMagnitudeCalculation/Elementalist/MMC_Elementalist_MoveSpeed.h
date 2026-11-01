// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MoveSpeed.generated.h"

/**
 * 元素法师 · 移动速度
 * 公式：380 + 敏捷 × 0.25（钳制：最低 0）
 * 移速最慢，靠位移技能弥补机动性。
 *
 * 单位：cm/s 量级，与 UE 中 MaxWalkSpeed 同量纲（引擎默认 600），
 * 便于后续按需接到 CharacterMovement 而不必再做换算。
 *
 * 为什么这样设计：公式里**没有等级项**——法师的机动性不随升级改善，
 * 只跟敏捷走（与 DodgeChance 一样是少数不含等级项的派生属性）。
 * 0.25/点敏捷也是四职业里偏低的成长，因此"靠位移技能弥补机动性"是它的玩法定位。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MoveSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造函数：只登记敏捷一个属性捕获；本公式不含等级项，因此也不解析角色等级 */
	UMMC_Elementalist_MoveSpeed();

	/**
	 * 由 GAS 的 ModifierMagnitude 求值流程调用：读取敏捷，
	 * 返回不小于 0 的移动速度值。
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 移动速度：380 + 敏捷 × 0.25
	// 移速最慢，靠位移技能弥补机动性。
	/** 基础移速 380：法师的起步机动性，只比坦克（370）略高，明显低于游侠（420） */
	static constexpr float BaseMoveSpeed = 380.f;

	/** 敏捷 → 移速：每 1 点敏捷 0.25 cm/s，法师唯一的移速成长来源（且不含等级项） */
	static constexpr float AgilityCoefficient = 0.25f;
};
