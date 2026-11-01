// Study Game Ability System _Hades
//
// @file  PlayerCharacterDefinition.cpp
// @brief 可玩角色定义的实现：把"角色"桥接到"职业数值配置"。

#include "Game/PlayerCharacterDefinition.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"

/**
 * 取出本角色所属职业在全局职业表里的默认配置。
 * 调用方：选人界面/角色预览需要展示职业默认数值时使用；
 * 战斗中的属性下发走 AAuraCharacterBase::FindClassDefaultInfo()，两者取到的是同一份数据。
 * 职业表未配置或该职业没有条目时返回 nullptr，调用方需要判空。
 */
const FCharacterClassDefaultInfo* UPlayerCharacterDefinition::FindClassDefaultInfo() const
{
	const UCharacterClassInfo* ClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(this);
	return ClassInfo ? ClassInfo->FindCharacterClassInformation(CharacterClass) : nullptr;
}
