// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_MagicalAttack.generated.h"

/**
 * 战士 · 魔法攻击力
 * 公式：2 + 智力 × 0.8 + 等级 × 0.3（钳制：最低 0）
 * 几乎无魔法输出，智力收益极低。
 */
UCLASS()
class AURA_API UMMC_Warrior_MagicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_MagicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 魔法攻击力：2 + 智力 × 0.8 + 等级 × 0.3
	// 几乎无魔法输出，智力收益极低。
	static constexpr float BaseMagicalAttack = 2.f;

	static constexpr float IntelligenceCoefficient = 0.8f;
	static constexpr float LevelCoefficient = 0.3f;
};
