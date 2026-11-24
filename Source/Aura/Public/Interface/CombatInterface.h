// Study Game Ability System _Hades
//
// 战斗接口：为"对象可以被战斗系统统一处理"提供一份契约。
// 凡参与战斗结算的对象（玩家角色、敌人等）都实现本接口，技能、MMC、UI 等模块
// 只需 Cast<ICombatInterface> 即可拿到等级与武器位置，而不必依赖 AuraCharacter /
// AuraEnemy 这些具体类型，从而避免上层代码与具体职业耦合。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"


USTRUCT(BlueprintType)
struct FMontageTag
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag = FGameplayTag();
};

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};


class AURA_API ICombatInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int32 GetCharacterLevel() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetAvatarActor();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FMontageTag> GetAttackMontages() const;
	
	/** 获取战斗插槽位置 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetCombatSocketLocation(const FGameplayTag& MontageTag) const;
	
	/** 获取战斗插槽 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FName GetCombatSocket(const FGameplayTag& MontageTag) const;
	
	/** 让角色转向指定目标点；在蓝图中实现， */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateFacingTarget(const FVector& TargetLocation);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UAnimMontage* GetHitReactMontage();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsDead() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Death();
};
