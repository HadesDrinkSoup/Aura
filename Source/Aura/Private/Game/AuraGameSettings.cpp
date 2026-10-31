// Study Game Ability System _Hades

#include "Game/AuraGameSettings.h"

#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Game/AuraCharacterRoster.h"

DEFINE_LOG_CATEGORY_STATIC(LogAuraSettings, Warning, All);

const UAuraGameSettings* UAuraGameSettings::Get()
{
	// CDO 只读：GetDefault 返回 const，避免下游拿到一个可以改配置的指针
	return GetDefault<UAuraGameSettings>();
}

const UCharacterClassInfo* UAuraGameSettings::GetCharacterClassInfo() const
{
	const UCharacterClassInfo* ClassInfo = CharacterClassInfo.LoadSynchronous();
	if (!ClassInfo)
	{
		UE_LOG(LogAuraSettings, Warning,
			TEXT("[Aura] UAuraGameSettings::CharacterClassInfo 未配置。")
			TEXT("请在 Project Settings → Game → Aura Game Settings 里指向 DA_CharacterClassInfo，")
			TEXT("否则所有角色都不会获得默认属性与技能。"));
	}
	return ClassInfo;
}

const UAuraCharacterRoster* UAuraGameSettings::GetCharacterRoster() const
{
	return CharacterRoster.LoadSynchronous();
}
