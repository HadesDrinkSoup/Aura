// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_AttackSpeed.generated.h"

/**
 * 游侠 · 攻击速度
 * 公式：110 + 敏捷 × 0.45 + 等级 × 0.2（钳制：50 ~ 250）
 * 攻速最高，高频普攻与技能衔接是核心玩法。
 */
UCLASS()
class AURA_API UMMC_Ranger_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_AttackSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 攻击速度：110 + 敏捷 × 0.45 + 等级 × 0.2
	// 攻速最高，高频普攻与技能衔接是核心玩法。
	static constexpr float BaseAttackSpeed = 110.f;

	static constexpr float AgilityCoefficient = 0.45f;
	static constexpr float LevelCoefficient = 0.2f;
};
