// Study Game Ability System _Hades

/**
 * @file AuraProjectileSpell.cpp
 * @brief 投射物技能：激活、客户端→服务器的发射请求转发，以及服务器侧生成投射物并注入伤害 GE Spec。
 *
 * 要点：
 *  - 生成投射物（SpawnActor）只能发生在服务器，客户端通过 Server RPC 请求；
 *  - 生成位置来自 ICombatInterface::GetCombatSocketLocation()（武器插槽世界坐标），
 *    朝向由"出生点 → 目标点"决定，本类不做射线检测/命中判定；
 *  - 用 SpawnActorDeferred + FinishSpawning，是为了在 BeginPlay（投射物在里面绑定 Overlap）之前
 *    就把 DamageEffectSpecHandle 填好，否则第一次命中时句柄还是空的；
 *  - 失败分支一律静默返回（除 ProjectileClass 未配置会打 Warning），不 EndAbility，
 *    避免技能因一次发射失败被取消。
 */

#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AuraGamepalyTags.h"
#include "Actor/AuraProjectile.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Interface/CombatInterface.h"

/**
 * 技能激活入口，由 GAS 在 TryActivateAbility 成功、CommitAbility 之后调用。
 * 本类保持空转（只调 Super）：目标点由蓝图里的 TargetDataUnderMouse 任务异步采集，
 * 采集完成后蓝图再调用 SpawnProjectile——所以 C++ 侧既不取目标数据也不生成投射物，
 * 这里没有"激活即发射"的逻辑。
 */
void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

/**
 * 蓝图调用入口：向目标点发射投射物。
 * 本函数只做"端侧分流"，真正的生成在 SpawnProjectileOnServer。
 *
 * @param ProjectileTargetLocation 目标点的世界坐标，仅用于计算投射物朝向（不做命中判定）。
 *
 * 失败/降级分支：
 *  - AvatarActor 为空（技能还没 InitActorInfo 或已销毁）→ 静默返回；
 *  - 客户端且 ASC 不是 UAuraAbilitySystemComponent（Cast 失败）→ 请求被丢弃，表现为"按了没反应"，
 *    因为当前只有 Aura 自己的 ASC 带 ServerSpawnProjectile 这个 RPC。
 */
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
			// 带上本次激活的 SpecHandle：服务器靠它找回同一个技能实例（并校验它确实在激活中），
			// 从而保证"客户端只能为一次真实存在的施法请求投射物"。
			ASC->ServerSpawnProjectile(GetCurrentAbilitySpecHandle(), ProjectileTargetLocation);
		}
		return;
	}

	// 服务器：直接生成
	SpawnProjectileOnServer(ProjectileTargetLocation);
}

/**
 * 唯一真正生成投射物的地方，只能在服务器执行。
 * 调用方有两条：本类的 SpawnProjectile（服务器/单机/主机本机路径），
 * 以及 UAuraAbilitySystemComponent::ServerSpawnProjectile_Implementation（远程客户端的请求转发）。
 *
 * 生成流程：
 *  1. 位置：ICombatInterface::GetCombatSocketLocation()（角色基类里是武器骨骼插槽的世界坐标，
 *     见 AuraCharacterBase.cpp:53），因此投射物从"手上/武器尖端"飞出，而不是角色胶囊中心；
 *  2. 朝向：出生点指向目标点的方向；
 *  3. Deferred Spawn + 填 DamageEffectSpecHandle + FinishSpawning（原因见函数内注释）。
 *
 * 失败分支：无服务器权限 / ProjectileClass 未配置 / Avatar 未实现 ICombatInterface /
 * 生成失败（IsValid 为假）时直接返回，不 EndAbility——宁可这一发没打出去，也不取消技能。
 */
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

	// 生成位置依赖 ICombatInterface：没有实现该接口的 Avatar 没有"发射点"概念，
	// 这里直接放弃，而不是退化到 Actor 原点（那会让投射物从脚底/角色中心冒出）。
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarActor);
	if (!CombatInterface) return;

	const FVector SpawnLocation = CombatInterface->GetCombatSocketLocation();
	// 朝向 = 出生点指向目标点。若目标点与出生点重合，该向量为零向量，
	// Rotation() 会得到全零旋转（投射物朝 +X 飞），属于极端情况，调用方保证目标点有效。
	const FRotator SpawnRotation = (ProjectileTargetLocation - SpawnLocation).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	
	// 用 Deferred 生成而不是普通 SpawnActor：投射物在 BeginPlay 里就绑定了 Overlap 回调，
	// 一旦命中就要 Apply DamageEffectSpecHandle；延迟生成让我们能在 BeginPlay 之前把该句柄写好，
	// 避免"刚生成就撞到东西、句柄还是空"的窗口期。
	// Owner 传角色本身：AAuraProjectile::BeginPlay 会 IgnoreActorWhenMoving(Owner)，
	// 防止投射物出生在武器/胶囊里立刻和自己人重叠；Instigator 传 Pawn，供伤害归因使用。
	// AlwaysSpawn：出生点与角色碰撞体重叠，默认的碰撞处理会拒绝生成或把位置挪开。
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass, 
		SpawnTransform, 
		GetOwningActorFromActorInfo(), 
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!IsValid(Projectile)) return;

	// 注意：这里没有校验 SourceASC / DamageEffectClass / SpecHandle。
	// 配置正确时（AvatarActor 实现了 IAbilitySystemInterface、DamageEffectClass 已填）
	// 一定拿得到有效句柄；若 DamageEffectClass 为空，句柄无效，
	// 投射物命中时会直接解引用 SpecHandle.Data（AuraProjectile.cpp:101），属于"配置错就崩"的路径。
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,GameplayTags.Damage, ScaledDamage);
	Projectile->DamageEffectSpecHandle = SpecHandle;
	
	// FinishSpawning 才真正把 Actor 放进世界并跑 BeginPlay；上面赋的 SpecHandle 已经就位。
	Projectile->FinishSpawning(SpawnTransform);
}
