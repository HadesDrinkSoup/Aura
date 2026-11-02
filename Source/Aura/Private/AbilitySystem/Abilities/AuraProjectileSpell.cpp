// Study Game Ability System _Hades

/**
 * @file AuraProjectileSpell.cpp
 * @brief 投射物技能：激活、服务器侧生成投射物并注入伤害 GE Spec。
 *
 * 要点：
 *  - 生成投射物（SpawnActor）只能发生在服务器；客户端那一次 SpawnProjectile 直接返回，
 *    由 GAS 复制过去的目标点驱动服务器端同一份技能实例生成；
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
#include "Interface/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

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
 * 本函数只做"端侧分流"：客户端直接返回，真正的生成统一在 SpawnProjectileOnServer。
 *
 * 客户端不需要、也不应该在这里补一个"请求"：
 *  - 技能是 Local Predicted，激活本身已由 GAS 复制到服务器（ServerTryActivateAbility）；
 *  - 目标点由 TargetDataUnderMouse 通过 ASC 的 ServerSetReplicatedTargetData 同步到服务器；
 *  - 服务器上同一份技能实例会再执行一次本函数（那一次 HasAuthority 为真），在那里生成。
 *
 * @param ProjectileTargetLocation 目标点的世界坐标，仅用于计算投射物朝向（不做命中判定）。
 *
 * 失败/降级分支：
 *  - AvatarActor 为空（技能还没 InitActorInfo 或已销毁）→ 静默返回；
 *  - 客户端 → 直接返回，不生成任何 Actor（客户端生成的东西不会被复制给别人）。
 */
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();

	// 客户端没有服务器权限：Actor 只能在服务器上生成，客户端生成的东西不会被复制给别人。
	// 目标点会由 TargetDataUnderMouse 同步到服务器，服务器端那份技能实例随后会再调用本函数。
	if (!AvatarActor && !AvatarActor->HasAuthority()) return;

	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: ProjectileClass 未设置，无法生成投射物"), *GetName());
		return;
	}

	// 生成位置依赖 ICombatInterface：没有实现该接口的 Avatar 没有"发射点"概念，
	// 这里直接放弃，而不是退化到 Actor 原点（那会让投射物从脚底/角色中心冒出）。
	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarActor);
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

	// 获取施法者的能力组件
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	// 创建伤害效果的 Spec
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
	// 获取项目的 GameplayTag 单例
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	// 根据技能等级计算实际伤害
	const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	// 向一个已经创建的 FGameplayEffectSpec（效果规格）中“写入”一个与 GameplayTag 绑定的浮点数值
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,GameplayTags.Damage, ScaledDamage);
	Projectile->DamageEffectSpecHandle = SpecHandle;
	
	// FinishSpawning 才真正把 Actor 放进世界并跑 BeginPlay；上面赋的 SpecHandle 已经就位。
	Projectile->FinishSpawning(SpawnTransform);
}