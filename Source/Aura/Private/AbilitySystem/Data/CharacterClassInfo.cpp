// Study Game Ability System _Hades


#include "AbilitySystem/Data/CharacterClassInfo.h"

const FCharacterClassDefaultInfo* UCharacterClassInfo::FindClassDefaultInfo(ECharacterClass CharacterClass) const
{
	return CharacterClassInformation.Find(CharacterClass);
}