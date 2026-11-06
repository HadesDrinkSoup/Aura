// Study Game Ability System _Hades
//
// @file  AuraGameModeBase.cpp
// @brief 战斗关 GameMode：持有全局静态数据（职业配置表）。
//
// 静态数据挂在本类的 BP 默认值上。
// 注意：GameMode 是服务器专属对象，客户端拿不到（见 AuraAbilitySystemLibrary 的说明）。

#include "Game/AuraGameModeBase.h"

#include "AbilitySystem/Data/CharacterClassInfo.h"

/**
 * 取职业配置表。
 * 这是"必须有"的配置：缺失时所有角色都拿不到默认属性与技能，
 * 所以这里给出可操作的错误提示（指明去哪个 BP 上配），而不是静默返回空。
 */
const UCharacterClassInfo* AAuraGameModeBase::GetCharacterClassInfo() const
{
	if (!CharacterClassInfo)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Aura] AAuraGameModeBase::CharacterClassInfo 未配置。")
			TEXT("请在 BP_AuraGameMode 的 Character Class Defaults 里指向 DA_CharacterClassInfo，")
			TEXT("否则所有角色都不会获得默认属性与技能。"));
	}
	return CharacterClassInfo;
}
