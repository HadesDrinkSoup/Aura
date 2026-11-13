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
	Tank,
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aura|Data|CharacterClass", meta = (DisplayName = "主属性GE"))
	TSubclassOf<UGameplayEffect> PrimaryAttributes;

	/** 派生属性 GE（14 个 MMC，必须是 Infinite，见 Docs/ModMagnitudeCalculation.md） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aura|Data|CharacterClass", meta = (DisplayName = "派生属性GE"))
	TSubclassOf<UGameplayEffect> DerivedAttributes;

	/** 池属性 GE（生命 / 法力） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aura|Data|CharacterClass", meta = (DisplayName = "池属性GE"))
	TSubclassOf<UGameplayEffect> PoolAttributes;

	/** 该职业出生时授予的技能 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aura|Data|CharacterClass", meta = (DisplayName = "初始技能"))
	TArray<TSubclassOf<UAuraGameplayAbility>> StartupAbilities;
};

/**
 * 职业配置表 —— 所有角色属性的唯一数据源
 *
 * 使用方式：
 *  1. 在 BP_AuraGameMode 的 `Character Class Defaults → Character Class Info` 里指向本资产（DA_CharacterClassInfo）。
 *  2. 角色 / 敌人 BP 只需要设置 `CharacterClass`（以及敌人的 Level），属性 GE 从这里下发。
 *  3. 敌人个体差异（精英、首领、特殊装备）不要复制整套 GE，而是在 BP 的
 *     `AdditionalInitialEffects` 里追加修正 GE。
 *
 * 注意：本表是「数值层」，玩家和敌人共用，只按职业下发 GE。
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	/** 职业 -> 默认配置 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Data|CharacterClass", meta = (DisplayName = "职业默认信息表"))
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;
	
	/**
	 * 通用能力：所有角色（玩家 / 敌人 / 任意职业）出生时都会授予的技能，例如受击反应。
	 *
	 * 与职业的 StartupAbilities 是并集关系，由 AAuraCharacterBase::GetCombinedStartupAbilities
	 * 合并进起始技能列表；即使 CharacterClass 在 CharacterClassInformation 里查不到也会下发。
	 * 若某个角色需要「额外一份」通用技能，直接配在角色 BP 的 StartupAbilities 上即可，不要复制本列表。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Data|CharacterClass", meta = (DisplayName = "通用技能"))
	TArray<TSubclassOf<UAuraGameplayAbility>> CommonAbilities;

	/**
	 * 灵属（元素亲和力）通用 GE：所有角色（玩家 / 敌人、任意职业）初始化时都会施加一次。
	 *
	 * 在该 GE 里给 `Affinity Attributes` 分类下的 Wood / Earth / Lightning / Water / Fire
	 * 各加 10（Duration Policy 必须是 Instant），本工程不内置该 GE，由蓝图创建后在
	 * DA_CharacterClassInfo 上指过来即可。
	 *
	 * 个体差异（某个敌人火抗 30、水抗 0 之类）不要改这里，而是在该敌人 BP 的
	 * `AdditionalElementAffinityEffects` 里追加修正 GE——它在本 GE 之后施加，因此可以
	 * 在 10 的基础上继续 Add / Multiply。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Data|CharacterClass", meta = (DisplayName = "通用元素亲和GE"))
	TSubclassOf<UGameplayEffect> CommonElementAffinity;

	/** 查找职业默认配置；找不到返回 nullptr（不崩溃） */
	const FCharacterClassDefaultInfo* FindCharacterClassInformation(const ECharacterClass& CharacterClass) const;
};
