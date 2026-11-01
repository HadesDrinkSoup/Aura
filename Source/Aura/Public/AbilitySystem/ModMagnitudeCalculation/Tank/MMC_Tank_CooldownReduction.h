// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_CooldownReduction.generated.h"

/**
 * 坦克 · 冷却缩减
 * 公式：0 + 智力 × 0.06 + 等级 × 0.02（钳制：0 ~ 40）
 * 冷却缩减最低：技能节奏最慢。
 */
UCLASS()
class AURA_API UMMC_Tank_CooldownReduction : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_CooldownReduction();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 冷却缩减：0 + 智力 × 0.06 + 等级 × 0.02
	// 冷却缩减最低：技能节奏最慢。
	static constexpr float BaseCooldownReduction = 0.f;

	static constexpr float IntelligenceCoefficient = 0.06f;
	static constexpr float LevelCoefficient = 0.02f;
};
