// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AuraCharacterSelectionSubsystem.generated.h"

class UPlayerCharacterDefinition;

/**
 * 本局选人结果（GameInstanceSubsystem）
 *
 * 为什么不是 GameMode，也不是 PlayerState：
 *  - 选人关和战斗关是**两个独立关卡**，OpenLevel 之后 GameMode 会重建，GameInstance 不会；
 *  - 单机 / 本地多人下「选择」是会话级状态，临时放在这里最省事。
 *  - 如果是联机（专用服务器 / 大厅），每个玩家各选各的，就必须改成 PlayerState 上
 *    带 Replicated 的属性，这个 Subsystem 只保留「本机玩家」的临时结果。
 */
UCLASS()
class AURA_API UAuraCharacterSelectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 选人界面点确认时调用（界面暂未实现，接口先留好） */
	UFUNCTION(BlueprintCallable, Category = "Aura|Selection")
	void SetSelectedCharacter(UPlayerCharacterDefinition* InCharacter);

	/** 清空选择（例如从菜单「重新开始」） */
	UFUNCTION(BlueprintCallable, Category = "Aura|Selection")
	void ClearSelectedCharacter();

	/** 当前选择；没选过返回 nullptr（不主动加载资产，界面里用请用同步加载的版本） */
	UFUNCTION(BlueprintPure, Category = "Aura|Selection")
	UPlayerCharacterDefinition* GetSelectedCharacter() const;

	/** 是否已经选过角色；战斗关可以据此判断要不要把玩家踢回选人关 */
	UFUNCTION(BlueprintPure, Category = "Aura|Selection")
	bool HasSelection() const;

	/** 战斗关 GameMode 用：选中的 Pawn 类；没选过 / 没配 PawnClass 时返回 nullptr */
	UClass* GetSelectedPawnClass() const;

	/** 战斗关 GameMode 用：选中的职业；没选过返回 false（回退到 BP 上配置的默认职业） */
	bool GetSelectedCharacterClass(ECharacterClass& OutCharacterClass) const;

private:
	UPROPERTY()
	TSoftObjectPtr<UPlayerCharacterDefinition> SelectedCharacter;
};
