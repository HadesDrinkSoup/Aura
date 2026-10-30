// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

class UAuraGameplayAbility;

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTagsDelegate, const FGameplayTagContainer&)

UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	
	void AddCharacterAbilities(const TArray<TSubclassOf<UAuraGameplayAbility>>& Abilities);
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void AbilityInputTagHeld(const FGameplayTag& InputTag);

	/**
	 * 客户端 -> 服务器：请求为某个已激活的技能生成投射物。
	 *
	 * 为什么 RPC 放在 ASC 上而不是 GameplayAbility 上：
	 * GameplayAbility 是普通 UObject，没有网络连接/NetGUID，声明在上面的 Server RPC 不会被正确路由；
	 * ASC 是挂在 PlayerState/Character 上的 ActorComponent，RPC 由它所在的 Actor 负责收发。
	 *
	 * 服务器收到后只承认"服务器上确实处于激活状态"的技能实例，客户端无法凭空伪造一次施法。
	 */
	UFUNCTION(Server, Reliable)
	void ServerSpawnProjectile(const FGameplayAbilitySpecHandle AbilitySpecHandle, const FVector& ProjectileTargetLocation);
	
	FEffectAssetTagsDelegate EffectAssetTagsDelegate;
	
protected:
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle) const;
};
