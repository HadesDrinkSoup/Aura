// Study Game Ability System _Hades

#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;

/**
 * 投射物技能基类：把"施放 → 取得目标点 → 服务器生成投射物 → 投射物命中时施加伤害"收成一条可复用的 C++ 技能链。
 *
 * 现有蓝图子类是 GA_FireBall（Content/Blueprint/AbilitySystem/Abilities/FireBall/GA_FireBall.uasset，
 * 该资产里引用了本类、UTargetDataUnderMouse 与 SpawnProjectile）。
 *
 * 网络分工（本类的核心约定）：
 *  - SpawnActor 只在服务器发生：客户端生成的 Actor 不会被复制给别人，表现也不会有；
 *  - 客户端只负责"请求"：SpawnProjectile 把目标点通过 ASC 的 Server RPC
 *    （UAuraAbilitySystemComponent::ServerSpawnProjectile）发给服务器，由服务器再调 SpawnProjectileOnServer；
 *  - 伤害不在本类结算：本类只把"伤害 GE 的 Spec 句柄"塞进投射物（AAuraProjectile::DamageEffectSpecHandle），
 *    真正的 ApplyGameplayEffectSpecToSelf 发生在投射物命中目标的服务器侧（见 AuraProjectile.cpp:99-102）。
 *
 * 目标点来源：本类不采集输入。蓝图在激活时先用 AbilityTask TargetDataUnderMouse 把鼠标命中的
 * ValidData 取出来，再把命中点作为 ProjectileTargetLocation 调 SpawnProjectile。
 */
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:
	/**
	 * 真正在服务器上生成投射物，是唯一会 SpawnActor 的地方。
	 *
	 * 两条调用路径：
	 * - 服务器本机（单机 / listen server 主机 / AI）：SpawnProjectile 里直接调用；
	 * - 远程客户端：客户端通过 ASC 的 Server RPC 把"发射请求"发到服务器，
	 *   服务器找到本次激活的技能实例后调用（见 UAuraAbilitySystemComponent::ServerSpawnProjectile）。
	 */
	void SpawnProjectileOnServer(const FVector& ProjectileTargetLocation) const;

protected:
	/**
	 * 技能激活入口（重写 UGameplayAbility::ActivateAbility）。
	 * 本类的实现目前只调用 Super，不在这里取目标数据、也不在这里生成投射物：
	 * 目标点由蓝图的 Latent 任务 TargetDataUnderMouse 异步采集，再由蓝图调用 SpawnProjectile 下发。
	 * 因此"激活即发射"这条时序完全写在蓝图图里，C++ 侧不参与。
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	/**
	 * 蓝图调用：向目标点发射投射物。
	 *
	 * - 服务器：立即生成；
	 * - 客户端：没有服务器权限，不能自己生成，改为把请求转发给服务器（Server RPC）。
	 */
	UFUNCTION(BlueprintCallable, Category="ProjectileSpell")
	void SpawnProjectile(const FVector& ProjectileTargetLocation);
	
	/**
	 * 要生成的投射物类（AAuraProjectile 的子类，通常是一个蓝图投射物）。
	 * 来源：技能蓝图（GA_FireBall）的 Class Defaults 里配置；EditAnywhere 才允许在蓝图里改，
	 * 但仍属于"类默认值"层面，不是每个投射物实例单独传参。
	 * 未配置时 SpawnProjectileOnServer 会打 Warning 并放弃生成，不会崩（见实现处分支）。
	 */
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category="ProjectileClass")
	TSubclassOf<AAuraProjectile> ProjectileClass;
	
	/**
	 * 命中时施加的伤害 GE 类（Instant / Duration 由 GE 资产自己决定）。
	 * 用法：SpawnProjectileOnServer 里 MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ...)，
	 * 得到的 Spec 句柄被塞给投射物，由投射物命中目标时 Apply。
	 * 等级取 GetAbilityLevel()，而技能授予时 Level 固定传 1（AuraAbilitySystemComponent.cpp:18），
	 * 所以当前无论角色几级，伤害 GE 的等级都是 1——要让伤害随角色等级成长，得改授予时的 Level
	 * 或把成长做进 GE 的 MMC / CurveTable。
	 *
	 * 注：Category 与 ProjectileClass 同名（编辑器里两者会挤在同一分组），
	 * 另外标记写成了 BluePrintReadOnly（P 大写）；UHT 仍按 BlueprintReadOnly 处理
	 * （生成的反射标记里带 CPF_BlueprintReadOnly），仅书写不统一，行为不受影响。
	 */
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category="ProjectileClass")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
