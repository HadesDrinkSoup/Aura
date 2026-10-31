// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_AttackSpeed.generated.h"

/**
 * 元素法师 · 攻击速度
 * 公式：95 + 敏捷 × 0.15 + 等级 × 0.1（钳制：50 ~ 250）
 * 基础攻速最低，普攻不是主要输出手段。
 */
UCLASS()
class AURA_API UMMC_Elementalist_AttackSpeed : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_AttackSpeed();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 攻击速度：95 + 敏捷 × 0.15 + 等级 × 0.1
	// 基础攻速最低，普攻不是主要输出手段。
	static constexpr float BaseAttackSpeed = 95.f;

	static constexpr float AgilityCoefficient = 0.15f;
	static constexpr float LevelCoefficient = 0.1f;
};
