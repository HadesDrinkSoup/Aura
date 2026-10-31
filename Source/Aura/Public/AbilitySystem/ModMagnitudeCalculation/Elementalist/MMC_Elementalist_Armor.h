// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Elementalist_Armor.generated.h"

/**
 * 元素法师 · 护甲
 * 公式：0 + 活力 × 0.8 + 力量 × 0.2 + 等级 × 0.15（钳制：最低 0）
 * 基础护甲为 0，只能获得极低的物理减免。
 */
UCLASS()
class AURA_API UMMC_Elementalist_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Elementalist_Armor();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 元素法师 · 护甲：0 + 活力 × 0.8 + 力量 × 0.2 + 等级 × 0.15
	// 基础护甲为 0，只能获得极低的物理减免。
	static constexpr float BaseArmor = 0.f;

	static constexpr float VigorCoefficient = 0.8f;
	static constexpr float StrengthCoefficient = 0.2f;
	static constexpr float LevelCoefficient = 0.15f;
};
