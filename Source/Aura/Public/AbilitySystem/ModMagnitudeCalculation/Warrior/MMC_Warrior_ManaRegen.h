// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_ManaRegen.generated.h"

/**
 * 战士 · 法力恢复
 * 公式：0.5 + 智力 × 0.05 + 等级 × 0.02（钳制：最低 0）
 * 回蓝最低，法力属于稀缺资源。
 */
UCLASS()
class AURA_API UMMC_Warrior_ManaRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	UMMC_Warrior_ManaRegen();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 法力恢复：0.5 + 智力 × 0.05 + 等级 × 0.02
	// 回蓝最低，法力属于稀缺资源。
	static constexpr float BaseManaRegen = 0.5f;

	static constexpr float IntelligenceCoefficient = 0.05f;
	static constexpr float LevelCoefficient = 0.02f;
};
