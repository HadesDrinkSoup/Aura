// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Engine/DataAsset.h"
#include "CharacterClassInfo.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Elementalist,
	Warrior,
	Ranger,
};

/**
 * 单个职业的默认配置
 *
 * 「职业」只描述属性公式与默认技能，不关心具体是谁（玩家 / 哥布林 / Boss）。
 * 新增一个职业 = 在本表里加一条 + 准备 3 个属性 GE（公式由 MMC 决定）。
 */
USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	/** 主要属性 GE（力量 / 智力 / 敏捷 / 活力） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;

	/** 派生属性 GE（14 个 MMC，必须是 Infinite，见 Docs/ModMagnitudeCalculation.md） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DerivedAttributes;

	/** 池属性 GE（生命 / 法力） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> PoolAttributes;

	/** 该职业出生时授予的技能 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UAuraGameplayAbility>> StartupAbilities;
};

/**
 * 职业配置表 —— 所有角色属性的唯一数据源
 *
 * 使用方式：
 *  1. 在 AAuraGameModeBase 上挂一个本资产（DA_CharacterClassInfo）。
 *  2. 角色 / 敌人 BP 只需要设置 `CharacterClass`（以及敌人的 Level），属性 GE 从这里下发。
 *  3. 敌人个体差异（精英、首领、特殊装备）不要复制整套 GE，而是在 BP 的
 *     `AdditionalInitialEffects` 里追加修正 GE。
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	/** 职业 -> 默认配置 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;

	/** 查找职业默认配置；找不到返回 nullptr（不崩溃） */
	const FCharacterClassDefaultInfo* FindClassDefaultInfo(ECharacterClass CharacterClass) const;
};
