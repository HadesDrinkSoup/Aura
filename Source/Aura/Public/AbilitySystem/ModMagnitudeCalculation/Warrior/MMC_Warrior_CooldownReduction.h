// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_CooldownReduction.generated.h"

/**
 * 战士 · 冷却缩减
 * 公式：0 + 智力 × 0.08 + 等级 × 0.03（钳制：0 ~ 40）
 * 冷却缩减最低，技能循环明显偏慢。
 */
UCLASS()
class AURA_API UMMC_Warrior_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_CooldownReduction();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
