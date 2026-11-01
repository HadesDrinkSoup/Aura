// Study Game Ability System _Hades


#include "AbilitySystem/Data/CharacterClassInfo.h"

const FCharacterClassDefaultInfo* UCharacterClassInfo::FindCharacterClassInformation(const ECharacterClass& CharacterClass) const
{
	return CharacterClassInformation.Find(CharacterClass);
}