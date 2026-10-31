// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_MagicalAttack.generated.h"

/**
 * 元素法师 · 魔法攻击力
 * 公式：12 + 智力 × 3.4 + 等级 × 1.5（钳制：最低 0）
 * 核心输出：智力收益三职业最高。
 */
UCLASS()
class AURA_API UMMC_Elementalist_MagicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_MagicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 魔法攻击力：12 + 智力 × 3.4 + 等级 × 1.5
	// 核心输出：智力收益三职业最高。
	static constexpr float BaseMagicalAttack = 12.f;

	static constexpr float IntelligenceCoefficient = 3.4f;
	static constexpr float LevelCoefficient = 1.5f;
};
