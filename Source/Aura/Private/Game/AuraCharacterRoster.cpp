// Study Game Ability System _Hades

#include "Game/AuraCharacterRoster.h"

#include "Game/PlayerCharacterDefinition.h"

UPlayerCharacterDefinition* UAuraCharacterRoster::GetCharacter(int32 Index) const
{
	if (!Characters.IsValidIndex(Index))
	{
		return nullptr;
	}
	return Characters[Index].LoadSynchronous();
}

TArray<UPlayerCharacterDefinition*> UAuraCharacterRoster::GetAllCharacters() const
{
	TArray<UPlayerCharacterDefinition*> Result;
	Result.Reserve(Characters.Num());

	for (const TSoftObjectPtr<UPlayerCharacterDefinition>& Entry : Characters)
	{
		if (UPlayerCharacterDefinition* Character = Entry.LoadSynchronous())
		{
			Result.Add(Character);
		}
	}

	// SortOrder 相同时按资产名排序，保证每次打开界面顺序一致
	Result.Sort([](const UPlayerCharacterDefinition& A, const UPlayerCharacterDefinition& B)
	{
		return A.SortOrder != B.SortOrder ? A.SortOrder < B.SortOrder : A.GetName() < B.GetName();
	});

	return Result;
}
