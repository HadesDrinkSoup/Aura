// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Ranger_PhysicalAttack.generated.h"

/**
 * 游侠 · 物理攻击力
 * 公式：8 + 力量 × 1.3 + 敏捷 × 2.4 + 等级 × 1.2（钳制：最低 0）
 * 敏捷是主要加成，力量提供基础底子。
 */
UCLASS()
class AURA_API UMMC_Ranger_PhysicalAttack : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Ranger_PhysicalAttack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 游侠 · 物理攻击力：8 + 力量 × 1.3 + 敏捷 × 2.4 + 等级 × 1.2
	// 敏捷是主要加成，力量提供基础底子。
	static constexpr float BasePhysicalAttack = 8.f;

	static constexpr float StrengthCoefficient = 1.3f;
	static constexpr float AgilityCoefficient = 2.4f;
	static constexpr float LevelCoefficient = 1.2f;
};
