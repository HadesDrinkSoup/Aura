// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interface/CombatInterface.h"
#include "AuraCharacter.generated.h"

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public ICombatInterface
{
	GENERATED_BODY()
public:
	AAuraCharacter();
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	virtual int32 GetCharacterLevel() override;

protected:
	virtual void InitActorInfo() override;

	/** 防止 PossessedBy 和 OnRep_PlayerState 重复初始化 */
	bool bActorInfoInitialized = false;
};