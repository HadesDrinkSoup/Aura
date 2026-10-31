// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_HealthRegen.generated.h"

/**
 * 元素法师 · 生命恢复
 * 公式：0.5 + 活力 × 0.08 + 等级 × 0.03（钳制：最低 0）
 * 基础回血最低，续航依赖法力与技能。
 */
UCLASS()
class AURA_API UMMC_Elementalist_HealthRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_HealthRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
