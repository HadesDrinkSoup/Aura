// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_HitChance.generated.h"

/**
 * 坦克 · 命中几率
 * 公式：79 + 敏捷 × 0.05 + 等级 × 0.12（钳制：0 ~ 100）
 * 命中最低：动作迟缓，靠站桩与范围技能弥补。
 */
UCLASS()
class AURA_API UMMC_Tank_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_HitChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 命中几率：79 + 敏捷 × 0.05 + 等级 × 0.12
	// 命中最低：动作迟缓，靠站桩与范围技能弥补。
	static constexpr float BaseHitChance = 79.f;

	static constexpr float AgilityCoefficient = 0.05f;
	static constexpr float LevelCoefficient = 0.12f;
};
