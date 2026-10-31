// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_Armor.generated.h"

/**
 * 游侠 · 护甲
 * 公式：1 + 活力 × 1 + 力量 × 0.35 + 等级 × 0.25（钳制：最低 0）
 * 护甲偏低，被近身时风险很高。
 */
UCLASS()
class AURA_API UMMC_Ranger_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_Armor();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
