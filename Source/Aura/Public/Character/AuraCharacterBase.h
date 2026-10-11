// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "AuraCharacterBase.generated.h"

class UGameplayAbility;
struct FGameplayEffectContextHandle;
class UGameplayEffect;
class UAuraAttributeSet;
class UAuraAbilitySystemComponent;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	virtual void InitActorInfo() {};
	virtual void InitDefaultAttributes();

	// 帮助创建和应用GameplayEffect类中的游戏效果规范
	static void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, UAbilitySystemComponent* ASC, const FGameplayEffectContextHandle& EffectContext, float Level = 1.f);
	
	void AddCharacterAbilities() const;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> PoolAttributes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> DerivedAttributes;
	
private:
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
};
