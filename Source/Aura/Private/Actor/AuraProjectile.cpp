// Study Game Ability System _Hades

/**
 * @file AuraProjectile.cpp
 * @brief 投射物实现：组件与碰撞搭建、飞行音效、命中检测、伤害结算与命中表现广播。
 *
 * 要点：
 *  - 权威分工：生成、命中检测、伤害结算、销毁全部只在服务器执行；
 *    客户端上的这份 Actor 只是复制体，用于显示位置、播放音效与命中特效。
 *    因此 Overlap 回调第一行就是 HasAuthority() 早退——客户端即使因为位置误差产生本地重叠也不会误伤。
 *  - 表现不给客户端"自己算"的机会，统一由 Multicast_OnHit 在服务端命中时广播：
 *    服务端 Destroy() 之后客户端的碰撞就不可靠了，靠本地碰撞播特效会漏播。
 *  - 去重手段是"命中即 Destroy()"，没有单独的 bHit 标志；Destroy 是帧末生效，
 *    所以严格意义上同一帧内多次 Overlap 都可能进入结算（详见回调处的说明）。
 *  - 伤害数值不在本文件产生：Spec 由技能在生成阶段创建并注入（DamageEffectSpecHandle），
 *    投射物只负责"把它施加给被命中的 ASC"。
 */

#include "Actor/AuraProjectile.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Aura/Aura.h"

/**
 * 构造：搭组件树并配碰撞。
 *
 * 顺序是硬约束，不能调换：
 *  1) SphereComponent 必须是根组件，因为 ProjectileMovementComponent 需要 UpdatedComponent，
 *     而 UpdatedComponent 默认取根组件；先建移动组件会拿到空指针、投射物原地不动。
 *     （历史问题：早期版本先建 ArrowComponent 再建 Sphere 并事后换根，导致组件树/朝向错乱。）
 *  2) 组件创建完再赋值 InitialSpeed 等参数，避免依赖组件注册顺序。
 */
AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);   // 让客户端同步位置

	// 1) 先创建根组件 SphereComponent（修复原来先创建 Arrow 再创建 Sphere 的问题）
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
	// 注意：下面两行都设置对象类型，后一行会覆盖前一行——实际生效的是 ECC_WorldDynamic。
	// 也就是说角色网格对 ECC_Projectile 的 Overlap 响应（见 AuraCharacterBase 构造函数）对投射物并不起作用，
	// 命中靠的是本组件对 Pawn/WorldDynamic/WorldStatic 的 Overlap 响应。
	SphereComponent->SetCollisionObjectType(ECC_Projectile);
	// QueryOnly：只做查询（Overlap/Sweep），不参与物理阻挡，也不会产生 Hit 事件
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	// 先全部忽略，再逐个打开需要的通道：白名单式配置，避免默认响应带来的意外重叠
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	// Pawn 通道覆盖角色胶囊体；角色网格同样是 Pawn 类型的碰撞体，因此人物会被判定命中
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 2) 再创建移动组件（此时 RootComponent 已存在）
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	// InitialSpeed = MaxSpeed：匀速直线，不会被自身加速逻辑改变速度
	ProjectileMovementComponent->InitialSpeed = 550.f;
	ProjectileMovementComponent->MaxSpeed = 550.f;
	// 无重力：俯视角弹道要指哪打哪，不能下坠
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	// 朝向跟随速度：模型/特效的 +X 方向始终指向飞行方向（命中法线也依赖这个朝向兜底）
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);

	// 3) 兜底生命周期，防止无限飞行造成泄漏
	// 打空（没命中任何东西）时由它负责回收；命中时通常在到期前就 Destroy 了
	InitialLifeSpan = 3.f;
}

/**
 * 启动阶段。
 * 服务器与客户端都会执行，因此这里的代码必须"两端都成立"：
 * 忽略 Owner、绑定委托、播放循环音效（专用服务器除外）。
 */
void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 忽略 Owner（避免刚生成就与发射者重叠）
	// 只影响"本组件移动时"的扫掠忽略，不阻止 Owner 自己撞上来，
	// 所以回调里还保留了一次 OtherActor == GetOwner() 的判断作为兜底。
	if (AActor* OwnerActor = GetOwner())
	{
		SphereComponent->IgnoreActorWhenMoving(OwnerActor, true);
	}

	// 碰撞绑定（服务端和客户端都绑定，但逻辑在回调里用 HasAuthority 区分）
	// 客户端也必须绑定：它靠这条回调里的 Multicast 收到表现；本类不手动解绑，
	// Actor 销毁时组件与委托一起析构，不会留下悬空绑定。
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereComponentBeginOverlap);

	// 飞行循环音效（专用服务器不播放）
	// bStopWhenAttachedToDestroyed=true：组件随 Actor 销毁时自动停声，避免声音残留
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

/**
 * 命中回调（绑定到 SphereComponent 的 OnComponentBeginOverlap）。
 *
 * 处理顺序刻意固定为：早退 → 过滤 → 计算命中点 → 广播表现 → 结算伤害 → 销毁。
 * 广播放在结算之前，是为了让"特效/音效"和"伤害"在同一次网络批次里发出，
 * 客户端看到的表现不会比伤害慢；即便伤害因目标无 ASC 而跳过，表现依然播放。
 *
 * 已知风险：
 *  - 没有 bHit 标志，去重只靠末尾的 Destroy()。Destroy() 是标记待销毁、帧末处理，
 *    同一帧内与多个 Actor 重叠时回调可能被调用多次（可对多个目标结算），
 *    需要严格单次命中时应在函数开头加一个 bool 成员并提前 return。
 *  - DamageEffectSpecHandle 未判空就解引用：若生成方漏赋值，会在此处崩溃。
 */
void AAuraProjectile::OnSphereComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 只在服务端处理命中逻辑（伤害/销毁），客户端等 Multicast
	// 客户端也会收到本地 Overlap（复制体也在移动），但 ApplyGameplayEffectSpecToSelf 需要权威，
	// 与其让它静默失败并刷引擎警告，不如在这里就返回。
	if (!HasAuthority())
	{
		return;
	}

	// 过滤自身与发射者
	// IsValid 兼顾"已标记待销毁"的对象；OtherActor == this 覆盖自身子组件互相重叠的情况
	if (!IsValid(OtherActor) || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	
	// 命中点与法线（部分情况 SweepResult 为空则用 Actor 位置兜底）
	// 兜底法线取飞行方向的反向，即"迎着投射物来的面"，特效朝向不会突然歪掉
	FVector ImpactLocation = GetActorLocation();
	FVector ImpactNormal   = -GetActorForwardVector();

	// 用"是否接近零"来判断结果有效性：非 Sweep 触发时 FHitResult 是默认构造的全零结构。
	// 代价是命中点恰好落在世界原点附近时会被误判为无效（实际游戏中几乎不会发生）。
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
	
	// 只有带 GAS 的目标才结算伤害：打在墙、地板、可破坏物上时这里静默跳过，但特效照播
	// （UAbilitySystemBlueprintLibrary 未在本文件显式 include，依赖其它头文件间接引入）
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
	}
	
	// 服务端销毁（会复制到客户端销毁 Actor）
	// 销毁后客户端的碰撞不再可信，因此命中表现必须在上面先广播出去。
	Destroy();
}

/**
 * 命中表现的多播实现。
 *
 * 服务器调用 Multicast_OnHit 时，服务器自己也会立即执行一份（NetMulticast 在服务端本地直接调用），
 * 所以这里不需要额外判断"是否是服务器"。
 * 用 Reliable 是因为：命中表现与一次伤害绑定、事件频率低（每次命中一次），
 * 丢包会导致"怪掉血了但没有任何特效"的观感问题。
 */
void AAuraProjectile::Multicast_OnHit_Implementation(const FVector& ImpactLocation, const FVector& ImpactNormal)
{
	// 命中特效
	// 用命中法线转朝向：贴花/火花朝向被击中的表面，而不是始终朝向摄像机
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
	// 置空是为了让紧接着的 Destroyed() 不再重复 Stop（两处都会走到，必须幂等）
	if (LoopingAudioComponent)
	{
		LoopingAudioComponent->Stop();
		LoopingAudioComponent = nullptr;
	}
}

/**
 * 销毁收尾。
 * 服务器与客户端都会调用（客户端是在收到销毁复制后）。
 * 这里的 Stop 与 Multicast_OnHit 里的 Stop 是重复的，属于有意为之的兜底：
 * Multicast 可能在 Actor 已被销毁的客户端上被丢弃，只靠它就可能出现"声音还在播"。
 */
void AAuraProjectile::Destroyed()
{
	// 兜底：无论 Multicast 是否到达，销毁时都停掉循环音效
	// 此时音频组件可能已随 Actor 反注册而停止，Stop 是空操作，可安全重复调用
	if (LoopingAudioComponent)
	{
		LoopingAudioComponent->Stop();
		LoopingAudioComponent = nullptr;
	}

	Super::Destroyed();
}
