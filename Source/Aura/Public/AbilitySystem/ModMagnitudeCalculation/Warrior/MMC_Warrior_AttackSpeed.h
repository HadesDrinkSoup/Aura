// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_AttackSpeed.generated.h"

/**
 * 战士 · 攻击速度
 * 公式：100 + 敏捷 × 0.2 + 等级 × 0.1（钳制：50 ~ 250）
 * 攻速中等，重视单次伤害而非攻击频率。
 */
UCLASS()
class AURA_API UMMC_Warrior_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_AttackSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 攻击速度：100 + 敏捷 × 0.2 + 等级 × 0.1
	// 攻速中等，重视单次伤害而非攻击频率。
	static constexpr float BaseAttackSpeed = 100.f;

	static constexpr float AgilityCoefficient = 0.2f;
	static constexpr float LevelCoefficient = 0.1f;
};
