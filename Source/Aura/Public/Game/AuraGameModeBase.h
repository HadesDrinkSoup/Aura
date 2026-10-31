// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

/**
 * 战斗关的游戏模式
 *
 * 职责只有一件：**按本局选人结果创建并初始化玩家 Pawn**。
 * 全局静态数据（职业表、可选角色名单）在 UAuraGameSettings 里；
 * 本局选择（玩家选了哪个角色）在 UAuraCharacterSelectionSubsystem 里。
 * 因此菜单关不需要继承本类，也不必重复配置任何数据资产。
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 按选人结果返回 Pawn 类；没选过则回退到 BP 上配的 DefaultPawnClass */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/**
	 * spawn 完成、Possess 之前，把选中的职业写到 Pawn 上。
	 * 这样随后的 PossessedBy → InitActorInfo → InitDefaultAttributes 才能按正确职业下发 GE。
	 */
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
};
