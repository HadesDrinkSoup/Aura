// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_CooldownReduction.generated.h"

/**
 * 游侠 · 冷却缩减
 * 公式：1 + 敏捷 × 0.1 + 智力 × 0.05（钳制：0 ~ 40）
 * 冷却缩减一般，主要来自敏捷。
 */
UCLASS()
class AURA_API UMMC_Ranger_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_CooldownReduction();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
