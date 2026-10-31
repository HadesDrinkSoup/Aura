// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MoveSpeed.generated.h"

/**
 * 元素法师 · 移动速度
 * 公式：380 + 敏捷 × 0.25（钳制：最低 0）
 * 移速最慢，靠位移技能弥补机动性。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MoveSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_MoveSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 移动速度：380 + 敏捷 × 0.25
	// 移速最慢，靠位移技能弥补机动性。
	static constexpr float BaseMoveSpeed = 380.f;

	static constexpr float AgilityCoefficient = 0.25f;
};
