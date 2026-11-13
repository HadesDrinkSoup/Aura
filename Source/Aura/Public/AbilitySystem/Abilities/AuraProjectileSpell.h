// Study Game Ability System _Hades

#pragma once
#include "CoreMinimal.h"
#include "AuraDamageGameplayAbility.h"
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
 *  - 客户端不额外"请求"：技能是 Local Predicted，激活由 GAS 复制到服务器；
 *    目标点通过 TargetDataUnderMouse 里的 ServerSetReplicatedTargetData（ASC 内置 Server RPC）
 *    同步过去，服务器上同一份技能实例再调用 SpawnProjectileOnServer 生成；
 *  - 伤害不在本类结算：本类只把"伤害 GE 的 Spec 句柄"塞进投射物（AAuraProjectile::DamageEffectSpecHandle），
 *    真正的 ApplyGameplayEffectSpecToSelf 发生在投射物命中目标的服务器侧（见 AuraProjectile.cpp:99-102）。
 *
 * 目标点来源：本类不采集输入。蓝图在激活时先用 AbilityTask TargetDataUnderMouse 把鼠标命中的
 * ValidData 取出来，再把命中点作为 ProjectileTargetLocation 调 SpawnProjectile。
 */ 
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
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
	 * - 客户端：直接返回（不生成、也不发额外请求）；落点由 TargetDataUnderMouse 同步到服务器，
	 *   服务器端那份技能实例会再执行一次本函数并在那里生成。
	 *
	 * 朝向：只使用目标点相对发射点的水平分量（Z 清 0），目标是"投射物始终在发射点所在平面内飞行"；
	 * 目标点的 Z 不参与朝向计算。若目标点与发射点水平重合，则退化到施法者的水平朝向。
	 */
	UFUNCTION(BlueprintCallable, Category="ProjectileSpell")
	void SpawnProjectile(const FVector& ProjectileTargetLocation);
	
	/**
	 * 要生成的投射物类（AAuraProjectile 的子类，通常是一个蓝图投射物）。
	 * 来源：技能蓝图（GA_FireBall）的 Class Defaults 里配置；EditAnywhere 才允许在蓝图里改，
	 * 但仍属于"类默认值"层面，不是每个投射物实例单独传参。
	 * 未配置时 SpawnProjectileOnServer 会打 Warning 并放弃生成，不会崩（见实现处分支）。
	 */
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "Aura|Ability|Projectile", meta = (DisplayName = "投射物类"))
	TSubclassOf<AAuraProjectile> ProjectileClass;
};
