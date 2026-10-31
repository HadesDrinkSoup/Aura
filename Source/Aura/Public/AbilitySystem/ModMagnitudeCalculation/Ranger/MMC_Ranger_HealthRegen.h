// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_HealthRegen.generated.h"

/**
 * 游侠 · 生命恢复
 * 公式：0.8 + 活力 × 0.1 + 等级 × 0.04（钳制：最低 0）
 * 回血中等，续航依赖脱战与闪避。
 */
UCLASS()
class AURA_API UMMC_Ranger_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_HealthRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
