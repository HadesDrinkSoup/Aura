// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AuraCharacterRoster.generated.h"

class UPlayerCharacterDefinition;

/**
 * 可选角色名单（DA_CharacterRoster）
 *
 * 选人界面的唯一数据源：遍历它 → 每项是一个 UPlayerCharacterDefinition。
 * 没有界面时它依然是「可玩角色的注册表」：GameMode 只在选人结果里取 PawnClass，
 * 不关心一共有几个角色。
 */
UCLASS(BlueprintType)
class AURA_API UAuraCharacterRoster : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 可选角色（可以在 Details 面板里点上下箭头调整顺序，这里只是列表顺序） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roster")
	TArray<TSoftObjectPtr<UPlayerCharacterDefinition>> Characters;

	UFUNCTION(BlueprintPure, Category = "Aura|Character")
	int32 Num() const { return Characters.Num(); }

	/** 按索引取（同步加载）；索引越界或加载失败返回 nullptr */
	UFUNCTION(BlueprintPure, Category = "Aura|Character")
	UPlayerCharacterDefinition* GetCharacter(int32 Index) const;

	/** 同步加载全部并按 SortOrder 升序返回（数量很小，选人界面可以直接用） */
	UFUNCTION(BlueprintCallable, Category = "Aura|Character")
	TArray<UPlayerCharacterDefinition*> GetAllCharacters() const;
};
