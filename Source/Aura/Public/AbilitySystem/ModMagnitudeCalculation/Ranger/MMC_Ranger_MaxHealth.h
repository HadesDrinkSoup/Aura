// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_MaxHealth.generated.h"

/**
 * 游侠 · 最大生命值
 * 公式：90 + 活力 × 9 + 力量 × 4 + 等级 × 9（钳制：最低 1）
 * 血量中等：靠闪避与走位生存，而不是硬抗伤害。
 */
UCLASS()
class AURA_API UMMC_Ranger_MaxHealth : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_MaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
