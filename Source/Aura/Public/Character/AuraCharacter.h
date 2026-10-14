// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()
public:
	AAuraCharacter();
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void PawnClientRestart() override;
	
	virtual int32 GetCharacterLevel() override;

protected:
	virtual void InitActorInfo() override;
	void InitOverlayWidget() const;

	/** 防止 PossessedBy 和 OnRep_PlayerState 重复初始化 */
	bool bActorInfoInitialized = false;
};