// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_CriticalChance.generated.h"

/**
 * 游侠 · 暴击几率
 * 公式：8 + 敏捷 × 0.2 + 智力 × 0.03（钳制：0 ~ 75）
 * 暴击核心职业：敏捷收益三职业最高。
 */
UCLASS()
class AURA_API UMMC_Ranger_CriticalChance : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_CriticalChance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 暴击几率：8 + 敏捷 × 0.2 + 智力 × 0.03
	// 暴击核心职业：敏捷收益三职业最高。
	static constexpr float BaseCriticalChance = 8.f;

	static constexpr float AgilityCoefficient = 0.2f;
	static constexpr float IntelligenceCoefficient = 0.03f;
};
