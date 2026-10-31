// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_DodgeChance.generated.h"

/**
 * 元素法师 · 闪避几率
 * 公式：2 + 敏捷 × 0.08（钳制：0 ~ 60）
 * 闪避成长最差，不靠敏捷生存。
 */
UCLASS()
class AURA_API UMMC_Elementalist_DodgeChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_DodgeChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 闪避几率：2 + 敏捷 × 0.08
	// 闪避成长最差，不靠敏捷生存。
	static constexpr float BaseDodgeChance = 2.f;

	static constexpr float AgilityCoefficient = 0.08f;
};
