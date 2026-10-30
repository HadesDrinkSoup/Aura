// Study Game Ability System _Hades

#include "Character/AuraCharacterBase.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
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

void AAuraCharacterBase::InitDefaultAttributes()
{
	if (!HasAuthority()) return;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	// Require ASC and at least one attribute set to apply
	if (!ASC || (!PrimaryAttributes && !PoolAttributes && !DerivedAttributes)) return;

	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	if (EffectContextHandle.IsValid() && EffectContextHandle.Get())
	{
		if (AController* OwnerController = GetController())
		{
			EffectContextHandle.Get()->AddInstigator(OwnerController, this);
		}
		EffectContextHandle.Get()->AddSourceObject(this);
	}
	
	// Apply Primary attributes (if present)
	if (PrimaryAttributes)
	{
		ApplyEffectToSelf(PrimaryAttributes, ASC, EffectContextHandle);
	}
	
	if (DerivedAttributes)
	{
		ApplyEffectToSelf(DerivedAttributes, ASC, EffectContextHandle);
	}
	
	// Apply Pool attributes (if present)
	if (PoolAttributes)
	{
		ApplyEffectToSelf(PoolAttributes, ASC, EffectContextHandle);
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
	if (!HasAuthority()) return;
	AuraAbilitySystemComponent->AddCharacterAbilities(StartupAbilities);
}
