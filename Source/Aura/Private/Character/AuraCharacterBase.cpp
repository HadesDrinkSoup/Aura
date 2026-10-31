// Study Game Ability System _Hades

#include "Character/AuraCharacterBase.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "GameplayEffect.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick  = false;
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), "WeaponHandSocket");
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AuraAbilitySystemComponent;
}

FVector AAuraCharacterBase::GetCombatSocketLocation()
{
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

const FCharacterClassDefaultInfo* AAuraCharacterBase::FindClassDefaultInfo() const
{
	const UCharacterClassInfo* ClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(this);
	return ClassInfo ? ClassInfo->FindClassDefaultInfo(CharacterClass) : nullptr;
}

TArray<TSubclassOf<UAuraGameplayAbility>> AAuraCharacterBase::GetCombinedStartupAbilities() const
{
	TArray<TSubclassOf<UAuraGameplayAbility>> Abilities = StartupAbilities;

	if (const FCharacterClassDefaultInfo* ClassDefaults = FindClassDefaultInfo())
	{
		for (const TSubclassOf<UAuraGameplayAbility>& Ability : ClassDefaults->StartupAbilities)
		{
			Abilities.AddUnique(Ability);
		}
	}

	return Abilities;
}

void AAuraCharacterBase::InitDefaultAttributes()
{
	if (!HasAuthority()) return;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 解析顺序：本 BP 显式配置的 GE（个体覆盖） > 职业配置表中的默认 GE
	const FCharacterClassDefaultInfo* ClassDefaults = FindClassDefaultInfo();
	const TSubclassOf<UGameplayEffect> PrimaryEffect =
		PrimaryAttributes ? PrimaryAttributes : (ClassDefaults ? ClassDefaults->PrimaryAttributes : nullptr);
	const TSubclassOf<UGameplayEffect> DerivedEffect =
		DerivedAttributes ? DerivedAttributes : (ClassDefaults ? ClassDefaults->DerivedAttributes : nullptr);
	const TSubclassOf<UGameplayEffect> PoolEffect =
		PoolAttributes ? PoolAttributes : (ClassDefaults ? ClassDefaults->PoolAttributes : nullptr);

	if (!PrimaryEffect && !DerivedEffect && !PoolEffect && AdditionalInitialEffects.IsEmpty())
	{
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	if (EffectContextHandle.IsValid() && EffectContextHandle.Get())
	{
		if (AController* OwnerController = GetController())
		{
			EffectContextHandle.Get()->AddInstigator(OwnerController, this);
		}
		EffectContextHandle.Get()->AddSourceObject(this);
	}

	// GE 等级 = 角色等级：主要属性 GE 用 CurveTable 时可以直接按等级取数值
	const float EffectLevel = static_cast<float>(FMath::Max(GetCharacterLevel(), 1));

	// Apply Primary attributes (if present)
	if (PrimaryEffect)
	{
		ApplyEffectToSelf(PrimaryEffect, ASC, EffectContextHandle, EffectLevel);
	}
	
	if (DerivedEffect)
	{
		// 派生属性 GE 是 Infinite（主属性变化时由 GAS 自动重算），
		// 重复初始化（例如重生）时先移除旧实例，否则修正会叠加。
		FGameplayEffectQuery Query;
		Query.EffectDefinition = DerivedEffect;
		for (const FActiveGameplayEffectHandle& Handle : ASC->GetActiveEffects(Query))
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}

		ApplyEffectToSelf(DerivedEffect, ASC, EffectContextHandle, EffectLevel);
	}
	
	// 个体差异（精英加成、装备、诅咒等）：在 Derived 之后、Pool 之前施加。
	// 放在 Pool 之前的原因：个体 GE 常常会改主要属性 / MaxHealth，
	// 这样池属性 GE 才能用最终的 MaxHealth/MaxMana 把生命/法力填满（否则会出现"满血上限变高但血条不满"）。
	for (const TSubclassOf<UGameplayEffect>& ExtraEffect : AdditionalInitialEffects)
	{
		ApplyEffectToSelf(ExtraEffect, ASC, EffectContextHandle, EffectLevel);
	}

	// Apply Pool attributes (if present)：最后按最终的 MaxHealth/MaxMana 填满池属性
	if (PoolEffect)
	{
		ApplyEffectToSelf(PoolEffect, ASC, EffectContextHandle, EffectLevel);
	}
}

void AAuraCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> EffectClass, UAbilitySystemComponent* ASC, const FGameplayEffectContextHandle& EffectContext, const float Level)
{
	if (!ASC || !EffectClass) return;

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, Level, EffectContext);
	if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AAuraCharacterBase::AddCharacterAbilities() const
{
	if (!HasAuthority() || !AuraAbilitySystemComponent) return;
	AuraAbilitySystemComponent->AddCharacterAbilities(GetCombinedStartupAbilities());
}
