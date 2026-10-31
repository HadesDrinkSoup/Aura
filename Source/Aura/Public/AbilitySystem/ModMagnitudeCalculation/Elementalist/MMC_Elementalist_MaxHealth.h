// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MaxHealth.generated.h"

/**
 * 元素法师 · 最大生命值
 * 公式：65 + 活力 × 7 + 力量 × 2 + 等级 × 7（钳制：最低 1）
 * 体魄最弱：血量主要来自等级，活力收益明显低于战士。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MaxHealth : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_MaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 最大生命值：65 + 活力 × 7 + 力量 × 2 + 等级 × 7
	// 体魄最弱：血量主要来自等级，活力收益明显低于战士。
	static constexpr float BaseHealth = 65.f;

	static constexpr float VigorCoefficient = 7.f;
	static constexpr float StrengthCoefficient = 2.f;
	static constexpr float LevelCoefficient = 7.f;
};
