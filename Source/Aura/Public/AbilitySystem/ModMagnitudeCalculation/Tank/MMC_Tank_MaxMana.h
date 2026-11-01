// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_MaxMana.generated.h"

/**
 * 坦克 · 最大法力值
 * 公式：15 + 智力 × 2 + 活力 × 1 + 等级 × 0.5（钳制：最低 0）
 * 法力池最小，几乎不靠技能输出。
 */
UCLASS()
class AURA_API UMMC_Tank_MaxMana : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_MaxMana();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 最大法力值：15 + 智力 × 2 + 活力 × 1 + 等级 × 0.5
	// 法力池最小，几乎不靠技能输出。
	static constexpr float BaseMana = 15.f;

	static constexpr float IntelligenceCoefficient = 2.f;
	static constexpr float VigorCoefficient = 1.f;
	static constexpr float LevelCoefficient = 0.5f;
};
