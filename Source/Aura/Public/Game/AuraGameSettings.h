// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AuraGameSettings.generated.h"

class UCharacterClassInfo;
class UAuraCharacterRoster;

/**
 * 项目级全局配置（Project Settings → Game → Aura Game Settings）
 *
 * 这里是「静态数据」的家：职业配置表、可选角色名单。
 * 它不属于任何关卡，也不依赖 GameMode，因此：
 *  - 菜单关 / 战斗关 / 客户端 / 编辑器工具都能读；
 *  - 切换关卡（OpenLevel）不会丢失；
 *  - 不需要为了选人界面在第二个 GameMode 上再配一遍。
 *
 * 注意：本类里的值是「配置」，运行时不要改（改了也不会存盘）。
 * 本局选择（玩家选了哪个角色）属于 UAuraCharacterSelectionSubsystem。
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Aura Game Settings"))
class AURA_API UAuraGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 便捷访问（CDO，只读）。用 GetDefault 而不是 GetMutableDefault：配置对象不该被运行时改写 */
	static const UAuraGameSettings* Get();

	/**
	 * 全局职业配置表（DA_CharacterClassInfo）。
	 * 所有角色 / 敌人的属性 GE 与默认技能都从它读取。
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Character", meta = (DisplayPriority = "1"))
	TSoftObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	/**
	 * 可选角色名单（DA_CharacterRoster）。
	 * 选人界面遍历它；暂时没有界面，接口先留好。
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Character", meta = (DisplayPriority = "2"))
	TSoftObjectPtr<UAuraCharacterRoster> CharacterRoster;

	/** 同步加载职业配置表；未配置时返回 nullptr 并打警告（属性会全部下发失败，必须配置） */
	const UCharacterClassInfo* GetCharacterClassInfo() const;

	/** 同步加载可选角色名单；未配置时返回 nullptr（可选功能，不报警告） */
	const UAuraCharacterRoster* GetCharacterRoster() const;
};
