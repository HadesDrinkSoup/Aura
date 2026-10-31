// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_DodgeChance.generated.h"

/**
 * 战士 · 闪避几率
 * 公式：3 + 敏捷 × 0.07（钳制：0 ~ 60）
 * 闪避成长最低，靠护甲而不是闪避生存。
 */
UCLASS()
class AURA_API UMMC_Warrior_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_DodgeChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
