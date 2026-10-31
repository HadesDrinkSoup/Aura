// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_DodgeChance.generated.h"

/**
 * 游侠 · 闪避几率
 * 公式：6 + 敏捷 × 0.18（钳制：0 ~ 60）
 * 闪避核心职业：敏捷收益最高。
 */
UCLASS()
class AURA_API UMMC_Ranger_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_DodgeChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
