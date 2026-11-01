// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_PhysicalAttack.generated.h"

/**
 * 坦克 · 物理攻击力
 * 公式：12 + 力量 × 2.8 + 活力 × 0.5 + 等级 × 1.1（钳制：最低 0）
 * 伤害偏低：靠力量维持基本威胁，主输出不在这里。
 */
UCLASS()
class AURA_API UMMC_Tank_PhysicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_PhysicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 物理攻击力：12 + 力量 × 2.8 + 活力 × 0.5 + 等级 × 1.1
	// 伤害偏低：靠力量维持基本威胁，主输出不在这里。
	static constexpr float BasePhysicalAttack = 12.f;

	static constexpr float StrengthCoefficient = 2.8f;
	static constexpr float VigorCoefficient = 0.5f;
	static constexpr float LevelCoefficient = 1.1f;
};
