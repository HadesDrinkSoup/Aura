// Study Game Ability System _Hades
//
// @file  AuraCharacterSelectionSubsystem.cpp
// @brief 本局选人结果的存取实现（GameInstance 级，跨关卡存活）。
//
// 设计取舍：选择结果用 TSoftObjectPtr 保存，主流程不主动加载资产；
// 只有战斗关 GameMode 真正需要用 PawnClass / CharacterClass 时才 LoadSynchronous。
// GetSelectedCharacter 用 Get() 而不是 Load，避免蓝图调用意外触发同步加载。

#include "Game/AuraCharacterSelectionSubsystem.h"

// TSoftClassPtr<AAuraCharacterBase>::LoadSynchronous 需要完整类型
#include "Character/AuraCharacterBase.h"
#include "Game/PlayerCharacterDefinition.h"

/**
 * 记录本局选中的角色（选人界面点确认时调用）。
 * 这里只保存软引用，不做任何加载/校验：传 nullptr 等价于清空。
 */
void UAuraCharacterSelectionSubsystem::SetSelectedCharacter(UPlayerCharacterDefinition* InCharacter)
{
	SelectedCharacter = InCharacter;
}

/** 清空选择，回到"没选过"的状态（战斗关可据此把玩家送回选人关） */
void UAuraCharacterSelectionSubsystem::ClearSelectedCharacter()
{
	SelectedCharacter.Reset();
}

/**
 * 取当前选择（不加载资产）。
 * 用 Get() 而不是 LoadSynchronous：如果资产尚未加载就当作"没选"，
 * 避免蓝图在 UI 刷新时触发意外的同步加载卡顿。
 * 需要一定拿到数据的地方请改用 GetSelectedPawnClass / GetSelectedCharacterClass。
 */
UPlayerCharacterDefinition* UAuraCharacterSelectionSubsystem::GetSelectedCharacter() const
{
	// Get() 只返回「已经加载」的资产；没加载过就当没选，避免 UI 里出现意外同步加载
	return SelectedCharacter.Get();
}

/**
 * 是否已经做过选择。
 * 只看软引用是否为空（IsNull 不会触发加载），因此可以在任意时机安全调用。
 */
bool UAuraCharacterSelectionSubsystem::HasSelection() const
{
	return !SelectedCharacter.IsNull();
}

/**
 * 战斗关 GameMode 用：返回选中角色对应的 Pawn 类（同步加载）。
 * 两级都可能为空：没选过 / 资产丢失 / 角色没配 PawnClass，
 * 此时返回 nullptr，由 GameMode 回退到 DefaultPawnClass。
 */
UClass* UAuraCharacterSelectionSubsystem::GetSelectedPawnClass() const
{
	const UPlayerCharacterDefinition* Definition = SelectedCharacter.LoadSynchronous();
	return Definition ? Definition->PawnClass.LoadSynchronous() : nullptr;
}

/**
 * 战斗关 GameMode 用：把选中角色所属职业写入 OutCharacterClass。
 * 返回 false 表示"没有可用的选择"，调用方应保留自己的默认职业（而不是被写成 Elementalist）。
 */
bool UAuraCharacterSelectionSubsystem::GetSelectedCharacterClass(ECharacterClass& OutCharacterClass) const
{
	const UPlayerCharacterDefinition* Definition = SelectedCharacter.LoadSynchronous();
	if (!Definition)
	{
		return false;
	}

	OutCharacterClass = Definition->CharacterClass;
	return true;
}
