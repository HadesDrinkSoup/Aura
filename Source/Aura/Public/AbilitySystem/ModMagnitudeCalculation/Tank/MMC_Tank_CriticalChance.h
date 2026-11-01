// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_CriticalChance.generated.h"

/**
 * 坦克 · 暴击几率
 * 公式：3 + 力量 × 0.04 + 敏捷 × 0.03（钳制：0 ~ 75）
 * 暴击最低：慢速重击，不追求爆发。
 */
UCLASS()
class AURA_API UMMC_Tank_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_CriticalChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 暴击几率：3 + 力量 × 0.04 + 敏捷 × 0.03
	// 暴击最低：慢速重击，不追求爆发。
	static constexpr float BaseCriticalChance = 3.f;

	static constexpr float StrengthCoefficient = 0.04f;
	static constexpr float AgilityCoefficient = 0.03f;
};
