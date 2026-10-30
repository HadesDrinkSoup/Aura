// Study Game Ability System _Hades


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "Actor/AuraProjectile.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Interface/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	// 客户端没有服务器权限：Actor 只能在服务器上生成，客户端生成的东西不会被复制给别人。
	// 注意这里不能直接 return —— 那样客户端玩家就永远放不出技能。
	// 正确做法是：客户端只负责"请求"，把目标点发给服务器，由服务器生成投射物。
	if (!AvatarActor->HasAuthority())
	{
		if (UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
		{
			ASC->ServerSpawnProjectile(GetCurrentAbilitySpecHandle(), ProjectileTargetLocation);
		}
		return;
	}

	// 服务器：直接生成
	SpawnProjectileOnServer(ProjectileTargetLocation);
}

void UAuraProjectileSpell::SpawnProjectileOnServer(const FVector& ProjectileTargetLocation) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	// 兜底：这个函数只允许在服务器执行
	if (!AvatarActor || !AvatarActor->HasAuthority()) return;

	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: ProjectileClass 未设置，无法生成投射物"), *GetName());
		return;
	}

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarActor);
	if (!CombatInterface) return;

	const FVector SpawnLocation = CombatInterface->GetCombatSocketLocation();
	const FRotator SpawnRotation = (ProjectileTargetLocation - SpawnLocation).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass, 
		SpawnTransform, 
		GetOwningActorFromActorInfo(), 
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!IsValid(Projectile)) return;

	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
	Projectile->DamageEffectSpecHandle = SpecHandle;
	
	Projectile->FinishSpawning(SpawnTransform);
}
