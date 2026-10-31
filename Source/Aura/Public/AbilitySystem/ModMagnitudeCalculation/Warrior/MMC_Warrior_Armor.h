// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_Armor.generated.h"

/**
 * 战士 · 护甲
 * 公式：5 + 活力 × 1.8 + 力量 × 1.3 + 等级 × 0.55（钳制：最低 0）
 * 护甲最高：活力 + 力量双加成，物理减伤核心。
 */
UCLASS()
class AURA_API UMMC_Warrior_Armor : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_Armor();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 护甲：5 + 活力 × 1.8 + 力量 × 1.3 + 等级 × 0.55
	// 护甲最高：活力 + 力量双加成，物理减伤核心。
	static constexpr float BaseArmor = 5.f;

	static constexpr float VigorCoefficient = 1.8f;
	static constexpr float StrengthCoefficient = 1.3f;
	static constexpr float LevelCoefficient = 0.55f;
};
