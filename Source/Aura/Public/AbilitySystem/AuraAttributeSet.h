// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

class ACharacter;

DECLARE_DELEGATE_RetVal(FGameplayAttribute, FAttributeDelegate);

USTRUCT(BlueprintType)
struct FEffectProperties
{
	GENERATED_BODY()
	FEffectProperties() {};
	
	UPROPERTY()
	FGameplayEffectContextHandle EffectContextHandle;
	UPROPERTY()
	UAbilitySystemComponent* InstigatorASC = nullptr;
	UPROPERTY()
	AActor* InstigatorAvatarActor = nullptr;
	UPROPERTY()
	APlayerController* InstigatorController = nullptr;
	UPROPERTY()
	ACharacter* InstigatorCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;
	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;
	UPROPERTY()
	APlayerController* TargetController = nullptr;
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/** 构造函数，初始化默认属性值 */
	UAuraAttributeSet();

	/** 注册需要网络复制的属性列表 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** 在属性改变前进行钳制 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	TMap<FGameplayTag, FAttributeDelegate> TagsToAttributesMap;

	// ============================================================================
	//  Pool Attributes（池属性）
	// ============================================================================
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Pool Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Health)
	
	UPROPERTY(ReplicatedUsing = OnRep_Mana, BlueprintReadOnly, Category = "Pool Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Mana)

	// ============================================================================
	//  Primary Attributes（重要属性）
	// ============================================================================
	
	UPROPERTY(ReplicatedUsing = OnRep_Strength, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Strength)
	
	UPROPERTY(ReplicatedUsing = OnRep_Intelligence, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Intelligence)
	
	UPROPERTY(ReplicatedUsing = OnRep_Agility, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Agility)
	
	UPROPERTY(ReplicatedUsing = OnRep_Vigor, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Vigor)
	
	// ============================================================================
	//  Derived Attributes（派生属性）
	// ============================================================================
	
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MaxHealth)
	
	UPROPERTY(ReplicatedUsing = OnRep_MaxMana, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MaxMana)
	
	UPROPERTY(ReplicatedUsing = OnRep_PhysicalAttack, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData PhysicalAttack;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, PhysicalAttack)
	
	UPROPERTY(ReplicatedUsing = OnRep_MagicalAttack, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MagicalAttack;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MagicalAttack)
	
	UPROPERTY(ReplicatedUsing = OnRep_Armor, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Armor)
	
	UPROPERTY(ReplicatedUsing = OnRep_MagicResistance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MagicResistance;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MagicResistance)
	
	UPROPERTY(ReplicatedUsing = OnRep_CriticalChance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData CriticalChance;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, CriticalChance)
	
	UPROPERTY(ReplicatedUsing = OnRep_HitChance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData HitChance;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, HitChance)
	
	UPROPERTY(ReplicatedUsing = OnRep_DodgeChance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData DodgeChance;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, DodgeChance)
	
	UPROPERTY(ReplicatedUsing = OnRep_AttackSpeed, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, AttackSpeed)
	
	UPROPERTY(ReplicatedUsing = OnRep_CooldownReduction, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData 	CooldownReduction;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, CooldownReduction)
	
	UPROPERTY(ReplicatedUsing = OnRep_HealthRegen, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData 	HealthRegen;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, HealthRegen)
	
	UPROPERTY(ReplicatedUsing = OnRep_ManaRegen, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData 	ManaRegen;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, ManaRegen)
	
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MoveSpeed)

	// ============================================================================
	//  OnRep 网络复制回调
	// ============================================================================

	/** Health 属性网络复制回调，属性值变化时自动调用 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	/** Mana 属性网络复制回调，属性值变化时自动调用 */
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;
	
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
	
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	
	UFUNCTION()
	void OnRep_Agility(const FGameplayAttributeData& OldAgility) const;
	
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;
	
	/** MaxHealth 属性网络复制回调，属性值变化时自动调用 */
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	
	/** MaxMana 属性网络复制回调，属性值变化时自动调用 */
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;
	
	UFUNCTION()
	void OnRep_PhysicalAttack(const FGameplayAttributeData& OldPhysicalAttack) const;
	
	UFUNCTION()
	void OnRep_MagicalAttack(const FGameplayAttributeData& OldMagicalAttack) const;
	
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;
	
	UFUNCTION()
	void OnRep_MagicResistance(const FGameplayAttributeData& OldMagicResistance) const;
	
	UFUNCTION()
	void OnRep_CriticalChance(const FGameplayAttributeData& OldCriticalChance) const;
	
	UFUNCTION()
	void OnRep_HitChance(const FGameplayAttributeData& OldHitChance) const;
	
	UFUNCTION()
	void OnRep_DodgeChance(const FGameplayAttributeData& OldDodgeChance) const;
	
	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed) const;
	
	UFUNCTION()
	void OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction) const;
	
	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const;
	
	UFUNCTION()
	void OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen) const;
	
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed) const;
	
private:
	static void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties);
};
