// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MoveSpeed.generated.h"

/**
 * 战士 · 移动速度
 * 公式：390 + 敏捷 × 0.3 + 力量 × 0.05（钳制：最低 0）
 * 移速中等，力量提供极少额外移速。
 */
UCLASS()
class AURA_API UMMC_Warrior_MoveSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_MoveSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 移动速度：390 + 敏捷 × 0.3 + 力量 × 0.05
	// 移速中等，力量提供极少额外移速。
	static constexpr float BaseMoveSpeed = 390.f;

	static constexpr float AgilityCoefficient = 0.3f;
	static constexpr float StrengthCoefficient = 0.05f;
};
