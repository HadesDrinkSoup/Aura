// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_PhysicalAttack.generated.h"

/**
 * 战士 · 物理攻击力
 * 公式：12 + 力量 × 3.2 + 敏捷 × 0.4 + 等级 × 1.5（钳制：最低 0）
 * 核心输出：力量收益四职业最高。
 */
UCLASS()
class AURA_API UMMC_Warrior_PhysicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_PhysicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 物理攻击力：12 + 力量 × 3.2 + 敏捷 × 0.4 + 等级 × 1.5
	// 核心输出：力量收益四职业最高。
	static constexpr float BasePhysicalAttack = 12.f;

	static constexpr float StrengthCoefficient = 3.2f;
	static constexpr float AgilityCoefficient = 0.4f;
	static constexpr float LevelCoefficient = 1.5f;
};
