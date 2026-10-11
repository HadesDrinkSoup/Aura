// Study Game Ability System _Hades

#include "AbilitySystem/AuraAttributeSet.h"

#include "AuraGamepalyTags.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


UAuraAttributeSet::UAuraAttributeSet()
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	
	/** 池属性 */
	
	FAttributeDelegate HealthDelegate;
	HealthDelegate.BindStatic(GetHealthAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Pool_Health, HealthDelegate);
	
	FAttributeDelegate ManaDelegate;
	ManaDelegate.BindStatic(GetManaAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Pool_Mana, ManaDelegate);
	
	/** 重要属性 */
	FAttributeDelegate StrengthDelegate;
	StrengthDelegate.BindStatic(GetStrengthAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Strength, StrengthDelegate);
	
	FAttributeDelegate IntelligenceDelegate;
	IntelligenceDelegate.BindStatic(GetIntelligenceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Intelligence, IntelligenceDelegate);
	
	FAttributeDelegate AgilityDelegate;
	AgilityDelegate.BindStatic(GetAgilityAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Agility, AgilityDelegate);

	FAttributeDelegate VigorDelegate;
	VigorDelegate.BindStatic(GetVigorAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Vigor, VigorDelegate);
	
	/** 派生属性 */
	FAttributeDelegate MaxHealthDelegate;
	MaxHealthDelegate.BindStatic(GetMaxHealthAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MaxHealth, MaxHealthDelegate);
	
	FAttributeDelegate MaxManaDelegate;
	MaxManaDelegate.BindStatic(GetMaxManaAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MaxMana, MaxManaDelegate);
	
	FAttributeDelegate PhysicalAttackDelegate;
	PhysicalAttackDelegate.BindStatic(GetPhysicalAttackAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_PhysicalAttack, PhysicalAttackDelegate);
	
	FAttributeDelegate MagicalAttackDelegate;
	MagicalAttackDelegate.BindStatic(GetMagicalAttackAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MagicalAttack, MagicalAttackDelegate);
	
	FAttributeDelegate ArmorDelegate;
	ArmorDelegate.BindStatic(GetArmorAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_Armor, ArmorDelegate);
	
	FAttributeDelegate MagicResistanceDelegate;
	MagicResistanceDelegate.BindStatic(GetMagicResistanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MagicResistance, MagicResistanceDelegate);
	
	FAttributeDelegate CriticalChanceDelegate;
	CriticalChanceDelegate.BindStatic(GetCriticalChanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_CriticalChance, CriticalChanceDelegate);
	
	FAttributeDelegate HitChanceDelegate;
	HitChanceDelegate.BindStatic(GetHitChanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_HitChance, HitChanceDelegate);
	
	FAttributeDelegate DodgeChanceDelegate;
	DodgeChanceDelegate.BindStatic(GetDodgeChanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_DodgeChance, DodgeChanceDelegate);
	
	FAttributeDelegate AttackSpeedDelegate;
	AttackSpeedDelegate.BindStatic(GetAttackSpeedAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_AttackSpeed, AttackSpeedDelegate);
	
	FAttributeDelegate CooldownReductionDelegate;
	CooldownReductionDelegate.BindStatic(GetCooldownReductionAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_CooldownReduction, CooldownReductionDelegate);
	
	FAttributeDelegate HealthRegenDelegate;
	HealthRegenDelegate.BindStatic(GetHealthRegenAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_HealthRegen, HealthRegenDelegate);
	
	FAttributeDelegate ManaRegenDelegate;
	ManaRegenDelegate.BindStatic(GetManaRegenAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_ManaRegen, ManaRegenDelegate);
	
	FAttributeDelegate MoveSpeedDelegate;
	MoveSpeedDelegate.BindStatic(GetMoveSpeedAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MoveSpeed, MoveSpeedDelegate);
}

/**
 * 注册需要网络复制的属性及其复制通知回调
 *
 * - COND_None: 无条件复制（无论属性值是否改变，每次都会同步）
 * - REP NOTIFY_OnChanged: 属性值改变时触发对应的 OnRep_* 回调
 *
 * 当前复制的属性：
 * - Health / MaxHealth: 生命值与最大生命值
 * - Mana / MaxMana: 魔法值与最大魔法值
 */
void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_OnChanged);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Agility, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_OnChanged);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalAttack, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MagicalAttack, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MagicResistance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalChance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HitChance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, DodgeChance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, AttackSpeed, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CooldownReduction, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MoveSpeed, COND_None, REPNOTIFY_OnChanged);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	FEffectProperties EffectProperties;
	SetEffectProperties(Data, EffectProperties);

	// 在 GE 执行完毕后对属性进行钳制
	// 这是正确钳制位置 —— 此时所有属性修改都已生效，
	// GetMaxHealth() / GetMaxMana() 返回的都是最终值
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	// 当 MaxHealth 降低时，重新钳制 Health 防止 CurrentHealth > MaxHealth
	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetMaxManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
}

// ----------------------------------------------------------------------------
// 属性复制回调（OnRep）
//
// 当属性在服务器端发生变化、同步到客户端时调用。
// GAMEPLAY ATTRIBUTE_REP NOTIFY 宏会自动处理：
//   1. 将新值同步到 AbilitySystemComponent
//   2. 触发属性变更委托，通知所有监听者（UI、GameplayEffect 等）
// ----------------------------------------------------------------------------

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Agility, OldAgility);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_PhysicalAttack(const FGameplayAttributeData& OldPhysicalAttack) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalAttack, OldPhysicalAttack);
}

void UAuraAttributeSet::OnRep_MagicalAttack(const FGameplayAttributeData& OldMagicalAttack) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MagicalAttack, OldMagicalAttack);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_MagicResistance(const FGameplayAttributeData& OldMagicResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MagicResistance, OldMagicResistance);
}

void UAuraAttributeSet::OnRep_CriticalChance(const FGameplayAttributeData& OldCriticalChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalChance, OldCriticalChance);
}

void UAuraAttributeSet::OnRep_HitChance(const FGameplayAttributeData& OldHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HitChance, OldHitChance);
}

void UAuraAttributeSet::OnRep_DodgeChance(const FGameplayAttributeData& OldDodgeChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, DodgeChance, OldDodgeChance);
}

void UAuraAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UAuraAttributeSet::OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CooldownReduction, OldCooldownReduction);
}

void UAuraAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegen, OldHealthRegen);
}

void UAuraAttributeSet::OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegen, OldManaRegen);
}

void UAuraAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties)
{
	// --- 获取事件源（Instigator）信息 ---
	EffectProperties.EffectContextHandle = Data.EffectSpec.GetContext();
	EffectProperties.InstigatorASC = EffectProperties.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (EffectProperties.InstigatorASC)
	{
		if (EffectProperties.InstigatorASC->AbilityActorInfo.IsValid() && EffectProperties.InstigatorASC->AbilityActorInfo->AvatarActor.IsValid())
		{
			EffectProperties.InstigatorAvatarActor = EffectProperties.InstigatorASC->AbilityActorInfo->AvatarActor.Get();
			EffectProperties.InstigatorCharacter = Cast<ACharacter>(EffectProperties.InstigatorAvatarActor);

			if (EffectProperties.InstigatorASC->AbilityActorInfo->PlayerController.IsValid())
			{
				EffectProperties.InstigatorController = EffectProperties.InstigatorASC->AbilityActorInfo->PlayerController.Get();
			}
		}
	}

	// --- 获取目标（Target）信息 ---
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		EffectProperties.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		EffectProperties.TargetCharacter = Cast<ACharacter>(EffectProperties.TargetAvatarActor);
		EffectProperties.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(EffectProperties.TargetAvatarActor);

		if (Data.Target.AbilityActorInfo->PlayerController.IsValid())
		{
			EffectProperties.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		}
	}
}
