// Study Game Ability System _Hades
//
// @file  AuraGameModeBase.cpp
// @brief 战斗关 GameMode：把「本局选中的角色」翻译成「实际 spawn 的 Pawn 与职业」。
//
// 整体分工：静态数据在 UAuraGameSettings（职业表 / 名册），本局选择在
// UAuraCharacterSelectionSubsystem（GameInstance 级，跨关卡存活），
// 本文件只负责在 spawn 阶段把两者取出来用上。

#include "Game/AuraGameModeBase.h"

#include "Character/AuraCharacterBase.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Game/AuraCharacterSelectionSubsystem.h"
#include "GameFramework/Pawn.h"

namespace
{
	/**
	 * 从任意 World 上下文取选人子系统。
	 * 之所以写成自由函数：GameMode 的两个重写点都需要它，
	 * 而 World → GameInstance → Subsystem 这条链每一步都可能为空（编辑器预览、无 GI 的长尾场景），
	 * 集中处理可以避免两处各写一遍判空。
	 */
	UAuraCharacterSelectionSubsystem* GetSelectionSubsystem(const AActor* Context)
	{
		const UWorld* World = Context ? Context->GetWorld() : nullptr;
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UAuraCharacterSelectionSubsystem>() : nullptr;
	}
}

/**
 * 决定"这个玩家该 spawn 哪个 Pawn 类"。
 * 优先用选人结果里的 PawnClass；没选过（或子系统/资产缺失）就退回 Super 的实现，
 * 也就是 BP 上配置的 DefaultPawnClass —— 保证在没走选人流程时关卡仍然可以正常启动。
 */
UClass* AAuraGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UAuraCharacterSelectionSubsystem* Selection = GetSelectionSubsystem(this))
	{
		if (UClass* SelectedPawnClass = Selection->GetSelectedPawnClass())
		{
			return SelectedPawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

/**
 * Pawn 生成完成后、Possess 之前，把选中的职业写到 Pawn 上。
 *
 * 时序很关键：PossessedBy → InitActorInfo → InitDefaultAttributes 会在
 * 紧接着的 Possess 流程里读取 CharacterClass，所以必须在这之前设置好，
 * 否则会按 BP 默认职业下发属性 GE。
 * 先调 Super 拿到 Pawn 再处理，保证与父类的生成行为（含生成失败返回空）保持一致。
 */
APawn* AAuraGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	APawn* NewPawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);

	if (AAuraCharacterBase* AuraPawn = Cast<AAuraCharacterBase>(NewPawn))
	{
		if (const UAuraCharacterSelectionSubsystem* Selection = GetSelectionSubsystem(this))
		{
			// 默认值只是占位：GetSelectedCharacterClass 返回 false 时不会写回 Pawn
			ECharacterClass SelectedClass = ECharacterClass::Elementalist;
			if (Selection->GetSelectedCharacterClass(SelectedClass))
			{
				AuraPawn->SetCharacterClass(SelectedClass);
			}
		}
	}

	return NewPawn;
}
