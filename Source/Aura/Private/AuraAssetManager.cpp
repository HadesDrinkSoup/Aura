// Study Game Ability System _Hades


#include "AuraAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "AuraGamepalyTags.h"
#include "Engine/Engine.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine)
	return *Cast<UAuraAssetManager>(GEngine->AssetManager);
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FAuraGameplayTags::InitializeNativeGameplayTags();
	//使用Taget数据所必需的
	UAbilitySystemGlobals::Get().InitGlobalData();
}
