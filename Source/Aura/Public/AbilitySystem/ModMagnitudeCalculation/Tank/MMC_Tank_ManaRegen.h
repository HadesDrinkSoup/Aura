// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_ManaRegen.generated.h"

/**
 * 坦克 · 法力恢复
 * 公式：0.3 + 智力 × 0.05 + 等级 × 0.01（钳制：最低 0）
 * 回蓝最低：法力只够开关键技能。
 */
UCLASS()
class AURA_API UMMC_Tank_ManaRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_ManaRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 法力恢复：0.3 + 智力 × 0.05 + 等级 × 0.01
	// 回蓝最低：法力只够开关键技能。
	static constexpr float BaseManaRegen = 0.3f;

	static constexpr float IntelligenceCoefficient = 0.05f;
	static constexpr float LevelCoefficient = 0.01f;
};
