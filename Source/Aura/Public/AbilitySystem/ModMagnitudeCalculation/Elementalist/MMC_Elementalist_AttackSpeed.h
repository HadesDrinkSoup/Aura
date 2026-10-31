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
};
