// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_HitChance.generated.h"

/**
 * 战士 · 命中几率
 * 公式：82 + 敏捷 × 0.08 + 等级 × 0.15（钳制：0 ~ 100）
 * 命中稳定偏高，保证近战技能不落空。
 */
UCLASS()
class AURA_API UMMC_Warrior_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_HitChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 命中几率：82 + 敏捷 × 0.08 + 等级 × 0.15
	// 命中稳定偏高，保证近战技能不落空。
	static constexpr float BaseHitChance = 82.f;

	static constexpr float AgilityCoefficient = 0.08f;
	static constexpr float LevelCoefficient = 0.15f;
};
