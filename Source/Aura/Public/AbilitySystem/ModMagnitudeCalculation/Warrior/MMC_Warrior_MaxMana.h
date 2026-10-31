// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MaxMana.generated.h"

/**
 * 战士 · 最大法力值
 * 公式：25 + 智力 × 3.5 + 活力 × 1 + 等级 × 2.5（钳制：最低 0）
 * 法力池最小，只够支撑少量战技。
 */
UCLASS()
class AURA_API UMMC_Warrior_MaxMana : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_MaxMana();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
