// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "GameFramework/Character.h"
#include "Interface/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UAuraGameplayAbility;
struct FGameplayEffectContextHandle;
class UGameplayEffect;
class UAuraAttributeSet;
class UAuraAbilitySystemComponent;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual FVector GetCombatSocketLocation() override;

	/** 职业的读写：BP 里配的是默认值，战斗关由 GameMode 在 Possess 之前按选人结果覆盖 */
	void SetCharacterClass(ECharacterClass InClass) { CharacterClass = InClass; }
	ECharacterClass GetCharacterClass() const { return CharacterClass; }
	
protected:
	virtual void InitActorInfo() {};
	virtual void InitDefaultAttributes();

	// 帮助创建和应用GameplayEffect类中的游戏效果规范
	static void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, UAbilitySystemComponent* ASC, const FGameplayEffectContextHandle& EffectContext, float Level = 1.f);
	
	void AddCharacterAbilities() const;

	/** 取全局职业配置表中本角色职业的默认配置；没配职业信息时返回 nullptr */
	const FCharacterClassDefaultInfo* FindClassDefaultInfo() const;

	/** 职业默认技能 + 本 BP 额外配置的技能（自动去重） */
	TArray<TSubclassOf<UAuraGameplayAbility>> GetCombinedStartupAbilities() const;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	FName WeaponTipSocketName;
	
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;

	/**
	 * 职业：决定使用哪套属性 GE 与默认技能。
	 * 属性 GE 本身在 Project Settings → Game → Aura Game Settings 的 DA_CharacterClassInfo 里配置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class")
	ECharacterClass CharacterClass = ECharacterClass::Elementalist;

	/**
	 * 可选：直接在本 BP 上覆盖职业的属性 GE（留空则使用职业配置表）。
	 * 通常只在「职业表里没有的特殊个体」上使用。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> PoolAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> DerivedAttributes;

	/**
	 * 个体差异：在职业 GE 之后追加施加的 GE。
	 * 例如「精英哥布林」用一个 Multiply MaxHealth 的 GE，而不必复制整套职业 GE。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalInitialEffects;
	
private:
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UAuraGameplayAbility>> StartupAbilities;
};
