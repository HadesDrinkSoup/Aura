// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Tank_Armor.generated.h"

/**
 * 坦克 · 护甲
 * 公式：8 + 活力 × 2.6 + 力量 × 1.4 + 等级 × 0.7（钳制：最低 0）
 * 护甲最高：活力与力量双加成，物理减伤的核心。
 */
UCLASS()
class AURA_API UMMC_Tank_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Tank_Armor();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 坦克 · 护甲：8 + 活力 × 2.6 + 力量 × 1.4 + 等级 × 0.7
	// 护甲最高：活力与力量双加成，物理减伤的核心。
	static constexpr float BaseArmor = 8.f;

	static constexpr float VigorCoefficient = 2.6f;
	static constexpr float StrengthCoefficient = 1.4f;
	static constexpr float LevelCoefficient = 0.7f;
};
