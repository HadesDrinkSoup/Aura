// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_MoveSpeed.generated.h"

/**
 * 坦克 · 移动速度
 * 公式：370 + 敏捷 × 0.2 + 活力 × 0.02（钳制：最低 0）
 * 移速最慢：重甲代价，靠堵位而不是追击。
 */
UCLASS()
class AURA_API UMMC_Tank_MoveSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_MoveSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 移动速度：370 + 敏捷 × 0.2 + 活力 × 0.02
	// 移速最慢：重甲代价，靠堵位而不是追击。
	static constexpr float BaseMoveSpeed = 370.f;

	static constexpr float AgilityCoefficient = 0.2f;
	static constexpr float VigorCoefficient = 0.02f;
};
