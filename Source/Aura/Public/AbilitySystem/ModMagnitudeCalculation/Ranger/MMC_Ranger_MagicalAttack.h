// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_MagicalAttack.generated.h"

/**
 * 游侠 · 魔法攻击力
 * 公式：4 + 智力 × 1.5 + 敏捷 × 0.4 + 等级 × 0.7（钳制：最低 0）
 * 魔法输出为辅，敏捷提供少量额外法强。
 */
UCLASS()
class AURA_API UMMC_Ranger_MagicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_MagicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 魔法攻击力：4 + 智力 × 1.5 + 敏捷 × 0.4 + 等级 × 0.7
	// 魔法输出为辅，敏捷提供少量额外法强。
	static constexpr float BaseMagicalAttack = 4.f;

	static constexpr float IntelligenceCoefficient = 1.5f;
	static constexpr float AgilityCoefficient = 0.4f;
	static constexpr float LevelCoefficient = 0.7f;
};
