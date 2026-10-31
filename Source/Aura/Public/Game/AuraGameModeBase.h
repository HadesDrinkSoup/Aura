// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class UCharacterClassInfo;

/**
 * 游戏模式
 *
 * 这里挂载全局职业配置表：所有角色 / 敌人的属性 GE 与默认技能都从它读取，
 * 新建敌人时只需要在 BP 里选职业 + 填等级，不必再逐个拖 GE。
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 全局职业配置表（DA_CharacterClassInfo） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UCharacterClassInfo* GetCharacterClassInfo() const { return CharacterClassInfo; }
};
