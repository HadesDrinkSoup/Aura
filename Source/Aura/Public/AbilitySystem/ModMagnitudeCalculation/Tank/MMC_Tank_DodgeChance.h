// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_DodgeChance.generated.h"

/**
 * 坦克 · 闪避几率
 * 公式：1 + 敏捷 × 0.06（钳制：0 ~ 60）
 * 闪避最低：不靠走位，靠血量和护甲硬吃。
 */
UCLASS()
class AURA_API UMMC_Tank_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_DodgeChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 闪避几率：1 + 敏捷 × 0.06
	// 闪避最低：不靠走位，靠血量和护甲硬吃。
	static constexpr float BaseDodgeChance = 1.f;

	static constexpr float AgilityCoefficient = 0.06f;
};
