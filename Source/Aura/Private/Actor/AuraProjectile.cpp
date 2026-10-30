// Study Game Ability System _Hades

#include "Actor/AuraProjectile.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Aura/Aura.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);   // 让客户端同步位置

	// 1) 先创建根组件 SphereComponent（修复原来先创建 Arrow 再创建 Sphere 的问题）
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionObjectType(ECC_Projectile);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 2) 再创建移动组件（此时 RootComponent 已存在）
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovementComponent->InitialSpeed = 550.f;
	ProjectileMovementComponent->MaxSpeed = 550.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);

	// 3) 兜底生命周期，防止无限飞行造成泄漏
	InitialLifeSpan = 3.f;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 忽略 Owner（避免刚生成就与发射者重叠）
	if (AActor* OwnerActor = GetOwner())
	{
		SphereComponent->IgnoreActorWhenMoving(OwnerActor, true);
	}

	// 碰撞绑定（服务端和客户端都绑定，但逻辑在回调里用 HasAuthority 区分）
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereComponentBeginOverlap);

	// 飞行循环音效（专用服务器不播放）
	if (LoopingSound && GetNetMode() != NM_DedicatedServer)
	{
		LoopingAudioComponent = UGameplayStatics::SpawnSoundAttached(
			LoopingSound,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			/*bStopWhenAttachedToDestroyed=*/true);
	}
}

void AAuraProjectile::OnSphereComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 只在服务端处理命中逻辑（伤害/销毁），客户端等 Multicast
	if (!HasAuthority())
	{
		return;
	}

	// 过滤自身与发射者
	if (!IsValid(OtherActor) || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	
	// 命中点与法线（部分情况 SweepResult 为空则用 Actor 位置兜底）
	FVector ImpactLocation = GetActorLocation();
	FVector ImpactNormal   = -GetActorForwardVector();

	if (!SweepResult.ImpactPoint.IsNearlyZero())
	{
		ImpactLocation = SweepResult.ImpactPoint;   // FVector_NetQuantize 隐式转 FVector
	}
	if (!SweepResult.ImpactNormal.IsNearlyZero())
	{
		ImpactNormal = SweepResult.ImpactNormal;
	}

	// 广播所有端：播放特效/音效、停止循环音效
	Multicast_OnHit(ImpactLocation, ImpactNormal);
	
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
	}
	
	// 服务端销毁（会复制到客户端销毁 Actor）
	Destroy();
}

void AAuraProjectile::Multicast_OnHit_Implementation(const FVector& ImpactLocation, const FVector& ImpactNormal)
{
	// 命中特效
	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			ImpactEffect,
			ImpactLocation,
			ImpactNormal.Rotation());
	}

	// 命中音效
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			ImpactLocation,
			ImpactNormal.Rotation());
	}

	// 停止飞行循环音效
	if (LoopingAudioComponent)
	{
		LoopingAudioComponent->Stop();
		LoopingAudioComponent = nullptr;
	}
}

void AAuraProjectile::Destroyed()
{
	// 兜底：无论 Multicast 是否到达，销毁时都停掉循环音效
	if (LoopingAudioComponent)
	{
		LoopingAudioComponent->Stop();
		LoopingAudioComponent = nullptr;
	}

	Super::Destroyed();
}