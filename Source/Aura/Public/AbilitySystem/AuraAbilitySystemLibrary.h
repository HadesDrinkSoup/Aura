// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UAttributesMenuController;
class UCharacterClassInfo;
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure)
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	UFUNCTION(BlueprintPure)
	static UAttributesMenuController* GetAttributesMenuController(const UObject* WorldContextObject);

	/** 取全局职业配置表（由 AAuraGameModeBase 上的 DA_CharacterClassInfo 提供） */
	UFUNCTION(BlueprintPure, Category = "Aura|Character")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
};
