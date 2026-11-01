// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_AttackSpeed.generated.h"

/**
 * 坦克 · 攻击速度
 * 公式：90 + 敏捷 × 0.15 + 等级 × 0.1（钳制：50 ~ 250）
 * 攻速最低：重甲盾牌，出手慢但单次结实。
 */
UCLASS()
class AURA_API UMMC_Tank_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_AttackSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 攻击速度：90 + 敏捷 × 0.15 + 等级 × 0.1
	// 攻速最低：重甲盾牌，出手慢但单次结实。
	static constexpr float BaseAttackSpeed = 90.f;

	static constexpr float AgilityCoefficient = 0.15f;
	static constexpr float LevelCoefficient = 0.1f;
};
