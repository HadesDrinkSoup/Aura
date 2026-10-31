// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_HealthRegen.generated.h"

/**
 * 战士 · 生命恢复
 * 公式：1.5 + 活力 × 0.18 + 等级 × 0.06（钳制：最低 0）
 * 回血最高，配合高血量形成持续作战能力。
 */
UCLASS()
class AURA_API UMMC_Warrior_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_HealthRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
