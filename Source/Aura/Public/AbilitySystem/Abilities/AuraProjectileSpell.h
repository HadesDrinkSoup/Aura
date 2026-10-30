// Study Game Ability System _Hades

#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;

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
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	/**
	 * 蓝图调用：向目标点发射投射物。
	 *
	 * - 服务器：立即生成；
	 * - 客户端：没有服务器权限，不能自己生成，改为把请求转发给服务器（Server RPC）。
	 */
	UFUNCTION(BlueprintCallable, Category="ProjectileSpell")
	void SpawnProjectile(const FVector& ProjectileTargetLocation);
	
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category="ProjectileClass")
	TSubclassOf<AAuraProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category="ProjectileClass")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
