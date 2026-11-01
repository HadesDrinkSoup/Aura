// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_MagicalAttack.generated.h"

/**
 * 坦克 · 魔法攻击力
 * 公式：1 + 智力 × 0.5 + 等级 × 0.1（钳制：最低 0）
 * 几乎零法强，对抗法术靠抗性而不是伤害。
 */
UCLASS()
class AURA_API UMMC_Tank_MagicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_MagicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 魔法攻击力：1 + 智力 × 0.5 + 等级 × 0.1
	// 几乎零法强，对抗法术靠抗性而不是伤害。
	static constexpr float BaseMagicalAttack = 1.f;

	static constexpr float IntelligenceCoefficient = 0.5f;
	static constexpr float LevelCoefficient = 0.1f;
};
