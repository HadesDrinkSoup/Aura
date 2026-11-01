// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_MaxHealth.generated.h"

/**
 * 坦克 · 最大生命值
 * 公式：200 + 活力 × 18 + 力量 × 9 + 等级 × 16（钳制：最低 1）
 * 血量最高：活力主导、力量辅助，纯粹用来扛伤害。
 */
UCLASS()
class AURA_API UMMC_Tank_MaxHealth : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_MaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 最大生命值：200 + 活力 × 18 + 力量 × 9 + 等级 × 16
	// 血量最高：活力主导、力量辅助，纯粹用来扛伤害。
	static constexpr float BaseHealth = 200.f;

	static constexpr float VigorCoefficient = 18.f;
	static constexpr float StrengthCoefficient = 9.f;
	static constexpr float LevelCoefficient = 16.f;
};
