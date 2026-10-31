// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_PhysicalAttack.generated.h"

/**
 * 元素法师 · 物理攻击力
 * 公式：3 + 力量 × 1 + 敏捷 × 0.3 + 等级 × 0.6（钳制：最低 0）
 * 几乎不依赖武器，物理攻击只作兜底输出。
 */
UCLASS()
class AURA_API UMMC_Elementalist_PhysicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_PhysicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 物理攻击力：3 + 力量 × 1 + 敏捷 × 0.3 + 等级 × 0.6
	// 几乎不依赖武器，物理攻击只作兜底输出。
	static constexpr float BasePhysicalAttack = 3.f;

	static constexpr float StrengthCoefficient = 1.f;
	static constexpr float AgilityCoefficient = 0.3f;
	static constexpr float LevelCoefficient = 0.6f;
};
