// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_MoveSpeed.generated.h"

/**
 * 游侠 · 移动速度
 * 公式：420 + 敏捷 × 0.55（钳制：最低 0）
 * 移速最快，风筝与拉扯是核心优势。
 */
UCLASS()
class AURA_API UMMC_Ranger_MoveSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_MoveSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 移动速度：420 + 敏捷 × 0.55
	// 移速最快，风筝与拉扯是核心优势。
	static constexpr float BaseMoveSpeed = 420.f;

	static constexpr float AgilityCoefficient = 0.55f;
};
