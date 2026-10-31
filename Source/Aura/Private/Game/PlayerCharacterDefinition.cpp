// Study Game Ability System _Hades

#include "Game/PlayerCharacterDefinition.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"

const FCharacterClassDefaultInfo* UPlayerCharacterDefinition::FindClassDefaultInfo() const
{
	const UCharacterClassInfo* ClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(this);
	return ClassInfo ? ClassInfo->FindClassDefaultInfo(CharacterClass) : nullptr;
}
