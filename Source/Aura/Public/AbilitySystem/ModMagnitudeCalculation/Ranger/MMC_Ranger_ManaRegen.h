// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_ManaRegen.generated.h"

/**
 * 游侠 · 法力恢复
 * 公式：1 + 智力 × 0.12 + 敏捷 × 0.04 + 等级 × 0.04（钳制：最低 0）
 * 回蓝中等，敏捷提供少量额外回复。
 */
UCLASS()
class AURA_API UMMC_Ranger_ManaRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_ManaRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
