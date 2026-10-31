// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_MaxMana.generated.h"

/**
 * 游侠 · 最大法力值
 * 公式：55 + 智力 × 6.5 + 活力 × 1.5 + 等级 × 4.5（钳制：最低 0）
 * 法力中等，足以支撑标记与位移类技能。
 */
UCLASS()
class AURA_API UMMC_Ranger_MaxMana : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_MaxMana();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
