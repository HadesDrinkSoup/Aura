// Study Game Ability System _Hades
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "AuraProjectile.generated.h"

// 前置声明：头文件里只出现指针/句柄，避免把 GAS、Niagara、音频等头文件扩散到所有包含者。
// 注意：USoundBase 没有在这里声明（ImpactSound/LoopingSound 直接用了它），
// 其类型定义依赖别的头文件间接引入；若某次重构后编译报"USoundBase 未定义"，需补 include。
struct FGameplayEffectSpecHandle;
class UNiagaraSystem;
// UArrowComponent 目前没有任何成员使用它：早期版本用 Arrow 做朝向可视化，改成 Sphere 作为根组件后残留。
class UArrowComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UAudioComponent;

/**
 * AAuraProjectile
 *
 * 投射物：技能的"飞行体"载体，负责位移、命中检测、伤害结算与命中表现。
 *
 * 生命周期（全部由服务器驱动，客户端只是被动复制 + 播表现）：
 *  1) 技能 UAuraProjectileSpell::SpawnProjectileOnServer 用 SpawnActorDeferred 生成，
 *     在 FinishSpawning 之前把 DamageEffectSpecHandle 塞进来（对应 ExposeOnSpawn）；
 *  2) BeginPlay：忽略 Owner、绑定 Overlap、起飞行循环音效；
 *  3) 飞行中由 UProjectileMovementComponent 每帧推进（bReplicates + SetReplicateMovement
 *     保证客户端看到同一轨迹）；
 *  4) 命中：OnSphereComponentBeginOverlap → 只在服务器结算伤害并 Destroy()，
 *     同一次调用里通过 Multicast_OnHit 让所有端播命中特效/音效；
 *  5) 结束：Destroyed() 兜底停掉循环音效（3 秒 InitialLifeSpan 是没命中时的兜底）。
 *
 * 网络分工要点：
 *  - 伤害只能用服务器的 ApplyGameplayEffectSpecToSelf，所以命中检测、伤害、销毁都在服务器；
 *    客户端那份投射物只是"看得见的复制体"，它的 Overlap 回调会在第一行直接 return。
 *  - DamageEffectSpecHandle 没有标 Replicated：客户端上它永远是空句柄，
 *    因为客户端从不使用它，复制它只会浪费带宽。
 *  - 表现走 NetMulticast RPC：服务端销毁 Actor 后，客户端本地那份也会被销毁，
 *    所以"命中特效"必须由 RPC 主动广播，不能依赖客户端的碰撞。
 *
 * 注意（易踩的坑）：
 *  - 本类没有"已命中"标志位（如 bHit）：去重完全依赖命中后立即 Destroy()。
 *    Destroy() 是把对象标记为待销毁、在帧末统一处理，同一帧内若与多个 Actor 同时产生
 *    Overlap，回调可能进入多次 → 可能对多个目标都结算伤害，甚至对同一目标结算两次。
 *    需要严格"只命中一次"时必须自己加一个 bool 提前 return。
 *  - 碰撞组件是 QueryOnly + 全 Overlap：不产生 Block/Hit 事件，因此本类只绑定了
 *    OnComponentBeginOverlap，没有 OnComponentHit，不存在"Hit 与 Overlap 重复触发"的问题；
 *    若将来改成 Block 并同时绑定 OnHit，就必须用标志位去重。
 */
UCLASS()
class AURA_API AAuraProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	/** 构造：建立 Sphere 根组件与投射物移动组件，配置碰撞与兜底寿命。不在此处绑定任何委托 */
	AAuraProjectile();

	/**
	 * 销毁收尾。
	 * 由引擎在 Destroy() 的末尾调用（服务器销毁复制到客户端后，客户端也会走到这里），
	 * 用来兜底停掉飞行循环音效：NetMulticast 的命中 RPC 可能因为 Actor 已在客户端被销毁而丢失，
	 * 只靠 Multicast_OnHit 停音效会出现"投射物没了但嗡嗡声还在"。
	 */
	virtual void Destroyed() override;

	/**
	 * 命中时施加给目标的伤害 GE Spec。
	 *
	 * 关键点：
	 *  - 由施法者在生成时创建（SourceASC->MakeOutgoingSpec(DamageEffectClass, 技能等级, 上下文)），
	 *    因此 Spec 里的等级、SetByCaller、Instigator 都来自施法者而不是投射物；
	 *  - meta=(ExposeOnSpawn=true) 表示它可以在 SpawnActorDeferred 之后、FinishSpawning 之前
	 *    （或通过 SpawnActor 的 ExposeOnSpawn 参数）赋值——这是唯一能在 BeginPlay 前注入数据的窗口；
	 *  - 没有标记 Replicated：客户端拿到的是空句柄。命中结算只在服务器发生，所以不需要同步；
	 *  - 命中处未判空句柄：若生成方忘了赋值，会在这里空指针崩溃（由 UAuraProjectileSpell 保证非空）。
	 */
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;
protected:
	/**
	 * 开始播放：忽略发射者、绑定 Overlap 委托、启动飞行循环音效。
	 * 绑定放在 BeginPlay 而不是构造函数，是因为 AddDynamic 需要组件实例已注册、
	 * 且在构造函数里绑定会被蓝图子类的实例化流程重复执行。
	 */
	virtual void BeginPlay() override;

	/**
	 * 命中（重叠）回调。委托签名必须与 FComponentBeginOverlapSignature 完全一致，
	 * 且函数必须是 UFUNCTION()，否则 AddDynamic 无法生成委托绑定代码。
	 *
	 * 只在服务器真正处理；客户端进入后立刻 return（它只负责显示复制过来的位置）。
	 * 服务器上：过滤后再广播表现、对目标 ASC 施加伤害 Spec、最后销毁自身。
	 * 注意 bFromSweep 为 false 时 SweepResult 是空结构，命中点需要用 Actor 位置兜底。
	 */
	UFUNCTION()
	void OnSphereComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 广播给所有端播放命中特效/音效（服务端调用） */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnHit(const FVector& ImpactLocation, const FVector& ImpactNormal);

private:
	/**
	 * 投射物移动组件：负责匀速直线飞行（InitialSpeed=MaxSpeed=550，无重力，朝向跟随速度）。
	 * 它不推动 RootComponent 之外的东西，SetUpdatedComponent 必须在根组件设置之后调用，
	 * 所以本类构造函数里先建 SphereComponent、再建它（顺序写反会拿到空的 UpdatedComponent）。
	 * InitialSpeed/MaxSpeed 在 C++ 里硬编码，蓝图子类仍可在组件面板覆盖（组件模板可编辑）。
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
	
	/**
	 * 碰撞与根组件：QueryOnly + 对 WorldDynamic/WorldStatic/Pawn 只响应 Overlap，
	 * 所以既不会挡住角色，也不会产生 Hit 事件——命中完全由 BeginOverlap 驱动。
	 * 用球体而不是胶囊/盒子，是为了让"擦过"也能判定命中，且不需要考虑旋转。
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;
	
	/** 命中特效（Niagara），在 Multicast_OnHit 里按命中点与法线朝上生成；留空则不播 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ImpactEffect;
	
	/** 命中音效（单发），Multicast_OnHit 里在命中点播放；留空则不播 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> ImpactSound;
	
	/** 飞行期间的循环音效（如呼啸声），BeginPlay 时挂到根组件上，命中/销毁时停止 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundBase> LoopingSound;

	/** 循环飞行音效组件（用于命中/销毁时停止） */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> LoopingAudioComponent;
};
