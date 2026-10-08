// Study Game Ability System _Hades


#include "AuraAssetManager.h"
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
}
