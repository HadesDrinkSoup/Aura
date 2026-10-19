// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};


class AURA_API ICombatInterface
{
	GENERATED_BODY()

public:
	virtual int32 GetCharacterLevel() {return 0;};
	virtual FVector GetCombatSocketLocation() {return FVector(0,0,0);};
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& TargetLocation);
};
