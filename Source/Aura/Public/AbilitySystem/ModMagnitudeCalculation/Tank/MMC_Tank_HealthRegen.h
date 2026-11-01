// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_HealthRegen.generated.h"

/**
 * 坦克 · 生命恢复
 * 公式：1 + 活力 × 0.12 + 等级 × 0.04（钳制：最低 0）
 * 回血次高：长时间站场，脱战后恢复快。
 */
UCLASS()
class AURA_API UMMC_Tank_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_HealthRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 生命恢复：1 + 活力 × 0.12 + 等级 × 0.04
	// 回血次高：长时间站场，脱战后恢复快。
	static constexpr float BaseHealthRegen = 1.f;

	static constexpr float VigorCoefficient = 0.12f;
	static constexpr float LevelCoefficient = 0.04f;
};
