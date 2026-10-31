// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UAttributesMenuController;
class UCharacterClassInfo;
class UAuraCharacterRoster;
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static UAttributesMenuController* GetAttributesMenuController(const UObject* WorldContextObject);

	/**
	 * 取全局职业配置表（UAuraGameSettings 上配置的 DA_CharacterClassInfo）。
	 * 配置是进程级静态数据，不按世界查找；WorldContextObject 只用于 BP 侧自动填上下文。
	 */
	UFUNCTION(BlueprintPure, Category = "Aura|Character", meta = (WorldContext = "WorldContextObject"))
	static const UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	/** 取可选角色名单（UAuraGameSettings 上配置的 DA_CharacterRoster）；选人界面用 */
	UFUNCTION(BlueprintPure, Category = "Aura|Character", meta = (WorldContext = "WorldContextObject"))
	static const UAuraCharacterRoster* GetCharacterRoster(const UObject* WorldContextObject);
};
