// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Engine/DataAsset.h"
#include "PlayerCharacterDefinition.generated.h"

class AAuraCharacterBase;
class UTexture2D;

/**
 * 一个「可选角色」的定义（每个可玩角色一个资产：DA_PlayerCharacter_Elementalist 之类）
 *
 * 为什么和 FCharacterClassDefaultInfo 分开：
 *  - 职业表是「数值层」，敌我共用，一个职业一条；
 *  - 可玩角色是「展示层」，只有选人界面和玩家出生会用到，且同一个职业可以有多个角色。
 *    用 TMap<ECharacterClass, ...> 表达不了「两个 Warrior 可选角色」。
 */
UCLASS(BlueprintType)
class AURA_API UPlayerCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 显示名（本地化文本，不要用 FString） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	/** 选人界面上的介绍文本 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	/** 立绘 / 头像 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 选人界面的排序权重，小的在前（TMap / 资产列表的顺序都不可靠，必须显式排序） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	int32 SortOrder = 0;

	/**
	 * 战斗关实际 spawn 的 Pawn 类，也是选人界面的预览模型来源。
	 * 用软引用：选人界面不会因为这一点把整张角色表拖进内存。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TSoftClassPtr<AAuraCharacterBase> PawnClass;

	/**
	 * 指向数值层：属性 GE / 职业技能仍由 DA_CharacterClassInfo 按这个枚举下发。
	 * 一个职业可以被多个角色复用，个体差异写在角色 BP 的 AdditionalInitialEffects 里。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	ECharacterClass CharacterClass = ECharacterClass::Elementalist;

	/** 默认是否解锁（以后接存档 / 解锁条件） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression")
	bool bUnlockedByDefault = true;

	/** 便捷：取本角色所属职业的默认配置（属性 GE / 职业技能）；没配职业表时返回 nullptr */
	const FCharacterClassDefaultInfo* FindClassDefaultInfo() const;
};
