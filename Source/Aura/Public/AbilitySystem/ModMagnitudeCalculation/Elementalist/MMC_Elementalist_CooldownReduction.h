// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_CooldownReduction.generated.h"

/**
 * 元素法师 · 冷却缩减
 * 公式：3 + 智力 × 0.25 + 等级 × 0.1（钳制：0 ~ 40）
 * 冷却缩减核心职业，智力收益最高。
 */
UCLASS()
class AURA_API UMMC_Elementalist_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_CooldownReduction();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
