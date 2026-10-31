// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MaxHealth.generated.h"

/**
 * 战士 · 最大生命值
 * 公式：140 + 活力 × 15 + 力量 × 9 + 等级 × 15（钳制：最低 1）
 * 血量最高：活力与力量双收益，是前排承伤的核心。
 */
UCLASS()
class AURA_API UMMC_Warrior_MaxHealth : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_MaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
