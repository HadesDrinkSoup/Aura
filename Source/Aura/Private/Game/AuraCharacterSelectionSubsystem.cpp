// Study Game Ability System _Hades

#include "Game/AuraCharacterSelectionSubsystem.h"

// TSoftClassPtr<AAuraCharacterBase>::LoadSynchronous 需要完整类型
#include "Character/AuraCharacterBase.h"
#include "Game/PlayerCharacterDefinition.h"

void UAuraCharacterSelectionSubsystem::SetSelectedCharacter(UPlayerCharacterDefinition* InCharacter)
{
	SelectedCharacter = InCharacter;
}

void UAuraCharacterSelectionSubsystem::ClearSelectedCharacter()
{
	SelectedCharacter.Reset();
}

UPlayerCharacterDefinition* UAuraCharacterSelectionSubsystem::GetSelectedCharacter() const
{
	// Get() 只返回「已经加载」的资产；没加载过就当没选，避免 UI 里出现意外同步加载
	return SelectedCharacter.Get();
}

bool UAuraCharacterSelectionSubsystem::HasSelection() const
{
	return !SelectedCharacter.IsNull();
}

UClass* UAuraCharacterSelectionSubsystem::GetSelectedPawnClass() const
{
	const UPlayerCharacterDefinition* Definition = SelectedCharacter.LoadSynchronous();
	return Definition ? Definition->PawnClass.LoadSynchronous() : nullptr;
}

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
