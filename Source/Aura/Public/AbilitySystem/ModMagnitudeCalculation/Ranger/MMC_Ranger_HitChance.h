// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_HitChance.generated.h"

/**
 * 游侠 · 命中几率
 * 公式：88 + 敏捷 × 0.18 + 等级 × 0.2（钳制：0 ~ 100）
 * 命中最高，远程攻击的稳定性是核心优势。
 */
UCLASS()
class AURA_API UMMC_Ranger_HitChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_HitChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 命中几率：88 + 敏捷 × 0.18 + 等级 × 0.2
	// 命中最高，远程攻击的稳定性是核心优势。
	static constexpr float BaseHitChance = 88.f;

	static constexpr float AgilityCoefficient = 0.18f;
	static constexpr float LevelCoefficient = 0.2f;
};
