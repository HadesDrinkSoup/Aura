// Study Game Ability System _Hades

/**
 * @file AuraAbilitySystemComponent.cpp
 * @brief ASC 实现：ActorInfo 初始化时挂接 GE 回调、技能授予、输入标签到技能事件的翻译、投射物生成请求。
 *
 * 要点：
 *  - 本类不设置复制模式，复制模式由持有者（AAuraPlayerState / AAuraEnemy）各自决定；
 *  - 输入是「标签驱动」而非「槽位驱动」：技能用 DynamicSpecSourceTags 携带输入标签，输入回调按标签匹配；
 *  - 所有 Server/Client RPC 的权威逻辑都留在服务器，客户端只发请求、只接收结果。
 */

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGamepalyTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

/**
 * 调用基类完成 Owner/Avatar 绑定，然后把本 ASC 挂到「GE 施加到自身」的引擎委托上。
 * 必须放在 Super 之后：OnGameplayEffectAppliedDelegateToSelf 的回调里会用到已绑定的 AbilityActorInfo。
 * 该委托在服务器与客户端都会触发（服务器施加 GE、客户端收到 GE 复制各一次），
 * 因此 ClientEffectApplied 是 Client RPC，由服务器这边的触发负责把标签送到拥有者客户端。
 * 重复调用会累积多份绑定（AddUObject 不去重），见头文件说明。
 */
void UAuraAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
}

/**
 * 逐一授予技能：每个技能类构造一个等级为 1 的 Spec，把该类上配置的 AbilityInputTag 写进
 * Spec 的动态来源标签（DynamicSpecSourceTags），再 GiveAbility 交给基类管理。
 *
 * 为什么标签写在 Spec 而不是技能类上：Spec 是按角色授予的实例数据，同一个技能类在不同角色
 * 上可以绑不同键位；输入处理函数遍历 GetActivatableAbilities() 时也只拿得到 Spec。
 * CastChecked 意味着列表里混入非 UAuraGameplayAbility 的类会直接触发断言崩溃（不是静默跳过）。
 * 输入标签为空时添加空标签不会报警，但该技能将永远无法被输入匹配到。
 */
void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UAuraGameplayAbility>>& Abilities)
{
	for (const TSubclassOf AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = CastChecked<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->AbilityInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

// -----------------------------------------------------------------------------
// 能力输入处理
// 三个函数由 AAuraPlayerController 的输入回调转发而来，分别对应
// Enhanced Input 的 Started / Triggered / Completed。
// 这里按 GameplayTag 匹配能力（Tag 在 AddCharacterAbilities 中写入 DynamicAbilityTags），
// 再调用基类 AbilitySpecInputPressed / AbilitySpecInputReleased 把输入事件派发给能力实例。
// -----------------------------------------------------------------------------

/**
 * 按下：只标记并派发输入，不主动激活技能（激活发生在 Held 分支）。
 * 每个匹配的 Spec 都会收到事件，因此「一个输入标签被多个技能复用」时它们会同时收到按下。
 * 空标签直接返回：空标签会匹配到所有未配置标签的 Spec，必须提前挡掉。
 */
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// 遍历所有已授予且可激活的能力，找到 DynamicAbilityTags 精确匹配 InputTag 的 Spec
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// 标记「按下」并派发给能力实例（基类已处理 InstancedPerExecution 的多实例遍历）
			AbilitySpecInputPressed(AbilitySpec);
			// 能力处于激活中时广播 InputPressed 复制事件，供 WaitInputPress 等 GameplayTask 监听
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, FPredictionKey());
			}
		}
	}
}

/**
 * 松开：清掉 Pressed 状态并把 InputReleased 派发给能力实例。
 * 这里不广播 InvokeReplicatedEvent（只有按下路径广播了 InputPressed），
 * 因此依赖「松开复制事件」的预测型 AbilityTask 拿不到 InputReleased 的复制通知；
 * 当前项目没有这类 Task，若有需要应在此补 InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, ...)。
 */
void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// 松开 → 基类将 InputPressed 置 false，并派发 InputReleased 给能力实例
			// （蓄力类技能通常在松开的瞬间触发真正施放）
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

/**
 * 按住：每帧由输入系统触发。
 * 第一次触发时技能通常还没激活，所以在此 TryActivateAbility；之后每帧只维持 Pressed 状态。
 * TryActivateAbility 自带 CanActivate 校验（冷却、消耗、Blocked 标签等），失败时静默返回，
 * 下一帧 Triggered 会再试，所以本函数不需要自己判断失败原因。
 */
void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// 持续按住 → 同样标记按下状态
			AbilitySpecInputPressed(AbilitySpec);
			// Triggered 每帧触发，首次触发时能力通常尚未激活，故在此激活
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

/**
 * Client RPC 的实现：在拥有者客户端上把 GE 的资产标签转交 UI 层。
 * 只取 AssetTags（GE 类上配置的"发生什么事"标签），不取 GrantedTags / OngoingTagRequirements，
 * 因为 UI 需要的是逐次事件，而状态类标签应该通过属性/标签委托去查询。
 * 广播前不做任何过滤，Message.* 的筛选发生在 UOverlayWidgetController 的订阅回调里。
 */
void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle) const
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTagsDelegate.Broadcast(TagContainer);
}

/**
 * Server RPC 的实现：校验请求并为「服务器上真正激活中」的投射物技能生成投射物。
 * 失败分支全部静默返回：
 *   - Spec 句柄无效（技能已被移除/句柄过期）：丢弃；
 *   - Spec 存在但没有任何激活中的实例（客户端提前请求、或技能已结束）：丢弃；
 *   - 激活中的实例不是 UAuraProjectileSpell（同一 ASC 上其它技能的句柄被冒充）：丢弃。
 * 生成动作委托给技能自身，保证"投射物只在一个地方生成"（生成参数、Muzzle 位置、朝向计算都在那里）。
 */
void UAuraAbilitySystemComponent::ServerSpawnProjectile_Implementation(const FGameplayAbilitySpecHandle AbilitySpecHandle, const FVector& ProjectileTargetLocation)
{
	// ---------------------------------------------------------------------
	// 服务器端校验：客户端只负责"请求"，真正的生成永远由服务器执行。
	// 1) 找到请求所对应的技能 Spec；
	// 2) 只接受服务器上正在激活中的技能实例（客户端无法伪造一次没有在服务器上激活的施法）；
	// 3) 交给技能自己生成，生成逻辑只保留在 UAuraProjectileSpell 一处。
	// ---------------------------------------------------------------------
	const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (!AbilitySpec) return;

	// InstancedPerExecution 下一个 Spec 可能同时存在多个实例，挑出真正处于激活状态的那个
	for (UGameplayAbility* AbilityInstance : AbilitySpec->GetAbilityInstances())
	{
		const UAuraProjectileSpell* ProjectileSpell = Cast<UAuraProjectileSpell>(AbilityInstance);
		if (ProjectileSpell && ProjectileSpell->IsActive())
		{
			ProjectileSpell->SpawnProjectileOnServer(ProjectileTargetLocation);
			return;
		}
	}
}

