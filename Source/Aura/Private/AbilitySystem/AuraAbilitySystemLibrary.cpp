// Study Game Ability System _Hades

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Game/AuraGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"

const UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// 唯一读取点：数据源在 Project Settings，而不是某个关卡的 GameMode。
	// WorldContextObject 只服务 BP 的隐式上下文（meta = WorldContext），配置本身是进程级的。
	return UAuraGameSettings::Get()->GetCharacterClassInfo();
}

const UAuraCharacterRoster* UAuraAbilitySystemLibrary::GetCharacterRoster(const UObject* WorldContextObject)
{
	return UAuraGameSettings::Get()->GetCharacterRoster();
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
	{
		if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAuraAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAuraAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return HUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

UAttributesMenuController* UAuraAbilitySystemLibrary::GetAttributesMenuController(const UObject* WorldContextObject)
{
	if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
	{
		if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAuraAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAuraAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return HUD->GetAttributesMenuController(WidgetControllerParams);
		}
	}
	return nullptr;
}
