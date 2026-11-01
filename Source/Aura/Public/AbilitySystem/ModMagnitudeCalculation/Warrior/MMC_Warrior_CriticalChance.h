// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_CriticalChance.generated.h"

/**
 * 战士 · 暴击几率
 * 公式：5 + 敏捷 × 0.12 + 力量 × 0.05（钳制：0 ~ 75）
 * 暴击次高：敏捷与力量双加成，仅次于游侠（L50 约 13）。
 */
UCLASS()
class AURA_API UMMC_Warrior_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_CriticalChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 暴击几率：5 + 敏捷 × 0.12 + 力量 × 0.05
	// 暴击次高：敏捷与力量双加成，仅次于游侠（L50 约 13）。
	static constexpr float BaseCriticalChance = 5.f;

	static constexpr float AgilityCoefficient = 0.12f;
	static constexpr float StrengthCoefficient = 0.05f;
};
