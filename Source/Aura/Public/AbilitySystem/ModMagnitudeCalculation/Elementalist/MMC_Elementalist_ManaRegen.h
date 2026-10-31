// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_ManaRegen.generated.h"

/**
 * 元素法师 · 法力恢复
 * 公式：3 + 智力 × 0.3 + 等级 × 0.1（钳制：最低 0）
 * 回蓝核心职业，支撑高频技能循环。
 */
UCLASS()
class AURA_API UMMC_Elementalist_ManaRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_ManaRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 法力恢复：3 + 智力 × 0.3 + 等级 × 0.1
	// 回蓝核心职业，支撑高频技能循环。
	static constexpr float BaseManaRegen = 3.f;

	static constexpr float IntelligenceCoefficient = 0.3f;
	static constexpr float LevelCoefficient = 0.1f;
};
