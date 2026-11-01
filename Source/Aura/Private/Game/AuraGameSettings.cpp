// Study Game Ability System _Hades
//
// @file  AuraGameSettings.cpp
// @brief 项目级配置的读取实现：把 UDeveloperSettings 里的软引用按需同步加载成对象。
//
// 这里所有函数都是 const 的"只读查询"：配置值来自 DefaultGame.ini（打 Config 标记），
// 运行时不允许改写，因此返回的都是 const 指针。

#include "Game/AuraGameSettings.h"

#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Game/AuraCharacterRoster.h"

// 独立日志分类：只在缺失关键配置时输出 Warning，便于在日志里单独过滤 Aura 配置问题
DEFINE_LOG_CATEGORY_STATIC(LogAuraSettings, Warning, All);

/**
 * 取 CDO（Class Default Object）。
 * 用 GetDefault 而不是 GetMutableDefault：拿到的是 const 指针，从类型层面阻止下游修改配置。
 * UDeveloperSettings 的 CDO 由引擎在启动时从 ini 加载，因此这里不需要额外初始化。
 */
const UAuraGameSettings* UAuraGameSettings::Get()
{
	// CDO 只读：GetDefault 返回 const，避免下游拿到一个可以改配置的指针
	return GetDefault<UAuraGameSettings>();
}

/**
 * 同步加载全局职业配置表（DA_CharacterClassInfo）。
 * 这是"必须有"的配置：缺失时所有角色都拿不到默认属性与技能，
 * 所以这里给出可操作的错误提示（指明去哪个 Project Settings 页面配），而不是静默返回空。
 * 用 LoadSynchronous 是因为属性初始化发生在 Actor 构造/初始化阶段，无法等待异步回调。
 */
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

/**
 * 同步加载可选角色名单（DA_CharacterRoster）。
 * 属于可选功能（目前没有选人界面），因此未配置时直接返回 nullptr，不打警告。
 */
const UAuraCharacterRoster* UAuraGameSettings::GetCharacterRoster() const
{
	return CharacterRoster.LoadSynchronous();
}
