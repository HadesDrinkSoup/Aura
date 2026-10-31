// Study Game Ability System _Hades

#include "Game/AuraGameModeBase.h"

#include "Character/AuraCharacterBase.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Game/AuraCharacterSelectionSubsystem.h"
#include "GameFramework/Pawn.h"

namespace
{
	UAuraCharacterSelectionSubsystem* GetSelectionSubsystem(const AActor* Context)
	{
		const UWorld* World = Context ? Context->GetWorld() : nullptr;
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		return GameInstance ? GameInstance->GetSubsystem<UAuraCharacterSelectionSubsystem>() : nullptr;
	}
}

UClass* AAuraGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UAuraCharacterSelectionSubsystem* Selection = GetSelectionSubsystem(this))
	{
		if (UClass* SelectedPawnClass = Selection->GetSelectedPawnClass())
		{
			return SelectedPawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* AAuraGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	APawn* NewPawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);

	if (AAuraCharacterBase* AuraPawn = Cast<AAuraCharacterBase>(NewPawn))
	{
		if (const UAuraCharacterSelectionSubsystem* Selection = GetSelectionSubsystem(this))
		{
			ECharacterClass SelectedClass = ECharacterClass::Elementalist;
			if (Selection->GetSelectedCharacterClass(SelectedClass))
			{
				AuraPawn->SetCharacterClass(SelectedClass);
			}
		}
	}

	return NewPawn;
}
