// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

class UAuraGameplayAbility;

/**
 * 「GE 资产标签（Asset Tags）」广播用的多播委托。
 * 参数是刚刚施加到本 ASC 上的 GE 携带的 AssetTags 集合（由 ClientEffectApplied_Implementation 取出后广播）。
 * 唯一订阅方是 UOverlayWidgetController::BindCallbacksToDependencies：
 * 它在回调里筛出 Message.* 标签，再按标签去 MessageWidgetDataTable 查行，驱动拾取物/升级等提示 UI。
 * 非动态多播（不是 UFUNCTION 蓝图事件），只能 C++ 侧 AddUObject/AddLambda 绑定。
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTagsDelegate, const FGameplayTagContainer&)

/**
 * Aura 的技能系统组件（ASC）——GAS 在每个「有角色」的实体上的运行时中枢：
 * 技能授予/激活、GE 施加与复制、属性聚合，全部由它驱动。
 *
 * 谁创建、复制模式由谁决定（本类自身不设置复制模式）：
 *  - 玩家：AAuraPlayerState 构造函数里创建并 SetIsReplicated(true)，复制模式 Mixed
 *    （GE 复制给所有客户端以便他人也能看到血条，GameplayTag 只复制给拥有者以省带宽）。
 *    放在 PlayerState 上的原因是角色死亡/重生时 ASC 与属性不销毁；Character 只是通过
 *    InitAbilityActorInfo(PlayerState, Character) 扮演 Avatar 角色。
 *  - 敌人：AAuraEnemy 构造函数里自建，复制模式 Minimal（属性/GE 只对拥有者相关客户端有意义，
 *    敌人血条由本地 WidgetComponent 显示）。
 *
 * 职责概览：
 *  - InitAbilityActorInfo：基类绑定 Owner/Avatar 之后，再把自己挂到「GE 施加到自身」的引擎委托上，
 *    使服务器施加的 GE 能通过 Client RPC 把 AssetTags 送到拥有者客户端；
 *  - AddCharacterAbilities：把技能类变成 AbilitySpec，并把输入标签写进 Spec 的 DynamicSpecSourceTags，
 *    之后由输入处理函数按标签找回该技能；
 *  - AbilityInputTagPressed/Released/Held：把 Enhanced Input 的输入标签翻译成 GAS 的输入事件与激活请求；
 *  - ServerSpawnProjectile：客户端请求 → 服务器校验 → 生成投射物（投射物的权威生成点只有一处：
 *    UAuraProjectileSpell::SpawnProjectileOnServer）；
 *  - EffectAssetTagsDelegate：对 UI 层暴露的 GE 标签广播口。
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	/**
	 * 绑定 Owner/Avatar 并挂接 GE 施加回调。
	 *
	 * 调用时机（两处，都晚于 ASC 创建、早于初始属性 GE 的施加）：
	 *  - 玩家：AAuraCharacter::InitActorInfo → 服务器 PossessedBy、客户端 OnRep_PlayerState；
	 *  - 敌人：AAuraEnemy::InitActorInfo → BeginPlay。
	 *
	 * 注意：这里用 AddUObject 追加绑定而未先 Remove，重复调用会累积多份绑定；
	 * 一旦走到第二次，ClientEffectApplied 就会被调用多次，UI 消息可能重复广播。
	 * 目前靠 AAuraCharacter 的 bActorInfoInitialized 单次保护规避，敌人侧则依赖 BeginPlay 只执行一次。
	 * @param InOwnerActor   ASC 的逻辑拥有者（玩家侧是 PlayerState，敌人侧是 Character 自身）
	 * @param InAvatarActor  实际在世界中表现的角色（玩家侧是 Character，敌人侧同 Owner）
	 */
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	/**
	 * 授予技能并把「输入标签」写进每个 Spec 的动态标签。
	 *
	 * 一次性授予语义：GiveAbility 只是把 Spec 追加进可激活技能列表并同步给客户端，
	 * 之后该技能一直存在，不需要每帧调用，也不应重复调用；这里没有去重，
	 * 同一列表调用两次会让同一个技能拥有两份 Spec（例如玩家重生后再次经 PossessedBy 进入时）。
	 * 只在服务器有实际效果（GiveAbility 在客户端是空实现），由 AAuraCharacterBase::AddCharacterAbilities 调起。
	 *
	 * 输入标签写到 DynamicSpecSourceTags 而不是能力类的成员变量上，是因为 Spec 是每个角色独立的数据，
	 * 同一技能类在不同角色上可以绑定不同的键位。
	 * @param Abilities 已合并去重的技能类列表（AAuraCharacterBase::GetCombinedStartupAbilities 的产物）
	 */
	void AddCharacterAbilities(const TArray<TSubclassOf<UAuraGameplayAbility>>& Abilities);

	/**
	 * 输入「按下」（Enhanced Input 的 Started）。
	 * 遍历所有已授予且当前可激活的技能 Spec，标签精确匹配 InputTag 的才处理：
	 * 标记 Pressed 并把输入派发给能力实例；若该技能已在激活中，额外广播一次 InputPressed 复制事件，
	 * 供 WaitInputPress 一类 AbilityTask 监听（未激活的技能不需要广播，因为它的激活流程还没开始）。
	 * @param InputTag 输入映射里配置的能力标签，必须在 AddCharacterAbilities 时写进过某个 Spec
	 */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * 输入「松开」（Enhanced Input 的 Completed）。
	 * 只调用 AbilitySpecInputReleased：基类会清掉 Pressed 标记并把 InputReleased 派发给能力实例。
	 * 蓄力/持续引导类技能通常在这一刻才真正结算效果，因此匹配不到任何 Spec 时什么都不发生。
	 * @param InputTag 与按下时同一个输入标签
	 */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/**
	 * 输入「持续按住」（Enhanced Input 的 Triggered，每帧触发）。
	 * 维持 Pressed 状态，并在技能尚未激活时发起 TryActivateAbility——
	 * 这是本项目的激活入口（按下函数只派发输入、不激活，符合 GAS 的示例用法）。
	 * 已在激活中的技能会被反复调用 AbilitySpecInputPressed，因此技能内部必须能容忍重复的输入通知。
	 * @param InputTag 与按下时同一个输入标签
	 */
	void AbilityInputTagHeld(const FGameplayTag& InputTag);

	/**
	 * 客户端 -> 服务器：请求为某个已激活的技能生成投射物。
	 *
	 * 为什么 RPC 放在 ASC 上而不是 GameplayAbility 上：
	 * GameplayAbility 是普通 UObject，没有网络连接/NetGUID，声明在上面的 Server RPC 不会被正确路由；
	 * ASC 是挂在 PlayerState/Character 上的 ActorComponent，RPC 由它所在的 Actor 负责收发。
	 *
	 * 服务器收到后只承认"服务器上确实处于激活状态"的技能实例，客户端无法凭空伪造一次施法。
	 * @param AbilitySpecHandle      客户端当时激活的那个技能 Spec 句柄；服务器找不到对应 Spec 时直接忽略
	 * @param ProjectileTargetLocation 目标点（世界坐标），由技能按鼠标/目标数据求出后传入
	 */
	UFUNCTION(Server, Reliable)
	void ServerSpawnProjectile(const FGameplayAbilitySpecHandle AbilitySpecHandle, const FVector& ProjectileTargetLocation);

	/**
	 * GE 资产标签广播口：每次「GE 施加到本 ASC 自身」且到达拥有者客户端时触发一次。
	 * 只包含 GE 类上配置的 AssetTags（不是 GrantedTags / 不是 Spec 的 GetAssetTags 之外的内容），
	 * 因此它表达的是「发生了什么（拾取/升级/受击提示）」而非「当前拥有什么状态」。
	 */
	FEffectAssetTagsDelegate EffectAssetTagsDelegate;

protected:
	/**
	 * 服务器 → 拥有者客户端：把刚施加的 GE 的 AssetTags 转发给本地 UI。
	 *
	 * 由 InitAbilityActorInfo 中绑定的 OnGameplayEffectAppliedDelegateToSelf 触发。
	 * 之所以要绕一趟 RPC：GE 的施加发生在服务器（GE Spec 只复制给相关客户端），
	 * 而提示型 UI 需要「本次施加带了哪些资产标签」这种逐次事件，属性复制本身给不出。
	 * 注意实现体（_Implementation）在客户端同样可能被本地委托路径调用一次，
	 * 因此同一标签在客户端存在被广播两次的可能，UI 侧需自行去重/幂等。
	 * @param AbilitySystemComponent 发起广播的 ASC（本对象，RPC 参数用于保持与引擎委托签名一致）
	 * @param EffectSpec             刚施加的 GE Spec，用 GetAllAssetTags 取出资产标签
	 * @param ActiveEffectHandle     该 GE 的活动实例句柄（当前实现未使用，留给需要追踪实例的 UI）
	 */
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle) const;
};
