// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActiveGameplayEffectHandle.h"
#include "AuraEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * FActiveGameplayEffectHandleArray
 *
 * TArray<FActiveGameplayEffectHandle> 的薄包装结构体。
 * UE 反射系统不支持 TArray 直接作为 TMap 的 Value 类型（UHT 会报错），
 * 必须包一层 USTRUCT 来绕过此限制。
 *
 * 补充：结构体本身没有行为，只是 ActiveEffectsMap 的 Value 载体。
 * FActiveGameplayEffectHandle 是"某个端本地生效的效果实例"的索引，不是网络可复制数据，
 * 因此这里的句柄只在施加效果的端（本类约定为服务器）上有意义，不要指望客户端能拿到。
 */
USTRUCT()
struct FActiveGameplayEffectHandleArray
{
	GENERATED_BODY()

	/** 该目标 ASC 上由本 Actor 施加、且仍需由本 Actor 负责移除的活跃句柄；不复制、不存档 */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> Handles;
};

/**
 * AAuraEffectActor
 *
 * 效果 Actor 基类，封装了 GAS 中 GameplayEffect 的施加与移除逻辑。
 *
 * 使用方式：
 * 1. 在蓝图中继承此类，配置效果数组及选项
 * 2. 将此 Actor 放置在关卡中
 * 3. 当其他 Actor（需拥有 AbilitySystemComponent）与之发生 Overlap 时，
 *    根据配置自动施加或移除效果
 *
 * 典型配置示例：
 * - 进入瞬回血：          OnEnterInstantEffects=[GE_Heal]
 * - 离开给 DoT：          OnExitDurationEffects=[GE_DoT]
 * - 永久光环区域：         OnEnterInfiniteEffects=[GE_Aura]  bRemoveOnExit=true
 * - 进入光环 + 离开移除再给另一个光环：
 *                          OnEnterInfiniteEffects=[GE_AuraA] bRemoveOnExit=true OnExitInfiniteEffects=[GE_BuffB]
 * - 一次性消耗品：          OnEnterInstantEffects=[GE_Potion] bDestroy=true
 *
 * @note Infinite 类型的效果施加后会自动追踪其句柄；bRemoveOnExit
 *       控制离开时是否移除已追踪的效果。本类没有"进入时先移除"的开关：
 *       OnBeginOverlap 只施加不清理，进入时的清理只能由 Duration/Infinite GE 自身的
 *       Stacking 规则或外部逻辑完成。
 *
 * 设计要点（读代码时容易误解的三点）：
 *  - 本类不创建任何碰撞组件，也没有在构造函数/cpp 里绑定 Overlap 委托。
 *    OnBeginOverlap / OnEndOverlap 是 BlueprintCallable，由蓝图子类在自己的
 *    Sphere/Box 组件的 OnComponentBeginOverlap、OnComponentEndOverlap 里调用，
 *    因此"重叠范围多大、是否只有服务器产生重叠"都由蓝图侧决定。
 *  - "Instant / Duration / Infinite"只是配置上的分组约定，运行时代码并不按组分支：
 *    六个数组走同一条 ApplyEffectToTarget 路径，唯一的运行时分叉是
 *    GE 自身的 DurationPolicy 是否为 Infinite（决定是否追踪句柄）。
 *    也就是说，把 Infinite GE 错填进 Instant 数组，它仍然会被施加并被追踪。
 *  - 效果施加依赖 ApplyGameplayEffectSpecToSelf，该接口要求 ASC 拥有权威；
 *    在客户端调用会直接失败（无效句柄 + 引擎警告），所以调用方必须保证只有服务器执行。
 */
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()

public:
	/** 构造：关闭 Tick。本类完全事件驱动（等蓝图/引擎调用 Overlap 回调），没有逐帧逻辑 */
	AAuraEffectActor();

protected:
	/**
	 * 向目标 Actor 施加指定的 GameplayEffect
	 *
	 * @param InstigatorActor      效果的发起者
	 * @param TargetActor          效果的目标（需拥有 AbilitySystemComponent）
	 * @param GameplayEffectClass  要施加的 GameplayEffect 类
	 *
	 * @note Infinite 类型效果会自动追踪句柄，供后续移除
	 *
	 * 行为细节：
	 *  - 通过 IAbilitySystemInterface 探测 ASC，而不是 Cast 到 ACharacter：
	 *    任何挂 ASC 的 Actor（玩家角色、敌人、召唤物）都能被接受，没有 ASC 的 Actor 静默跳过。
	 *  - 目标 ASC 为空、GE 类为空、Spec 句柄无效三种情况都只是 return，不报错、不销毁自身，
	 *    因此"把药剂放在没有 ASC 的装饰物上"不会刷屏，但也不会给出任何反馈。
	 *  - EffectContext 的 SourceObject = 本 Actor，EffectCauser = 本 Actor，
	 *    Instigator = InstigatorActor（为空时退化为本 Actor）。
	 *    这样 GE 上的标签判定/击杀归属能追溯到"哪瓶药水、谁碰的"。
	 *  - GE 等级固定取 ActorLevel，因此想让 GE 随时间成长只能改这个字段或换 GE 类。
	 *  - 该方法只施加、不判断是否重复施加：同一目标反复进出重叠区域会反复施加新实例。
	 */
	UFUNCTION(BlueprintCallable, Category="Applied Effects")
	void ApplyEffectToTarget(AActor* InstigatorActor, AActor* TargetActor, const TSubclassOf<UGameplayEffect>& GameplayEffectClass);

	/**
	 * Overlap 开始时的回调
	 * 依次执行：施加 OnEnterInstantEffects → OnEnterDurationEffects → OnEnterInfiniteEffects
	 *
	 * 由蓝图在组件的 OnComponentBeginOverlap 中调用；设计上只在服务器调用
	 * （效果施加需要 ASC 权威）。末尾若 bDestroy 为真会立即销毁自身，
	 * 因此"一次性消耗品"模式下本次 Overlap 之后不会再收到 OnEndOverlap。
	 */
	UFUNCTION(BlueprintCallable, Category="Applied Effects")
	void OnBeginOverlap(AActor* InstigatorActor, AActor* TargetActor);

	/**
	 * Overlap 结束时的回调
	 * 依次执行：若 bRemoveOnExit 则移除已追踪效果 → 施加 OnExitInstantEffects → OnExitDurationEffects → OnExitInfiniteEffects → 若 bDestroy 则销毁自身
	 *
	 * 顺序不可颠倒：先移除旧效果，才能让紧接着施加的 OnExitInfiniteEffects
	 * 不被本次 bRemoveOnExit 清掉（详见 cpp 实现处的注释）。
	 */
	UFUNCTION(BlueprintCallable, Category="Applied Effects")
	void OnEndOverlap(AActor* InstigatorActor, AActor* TargetActor);

	// ============================================================================
	// 配置属性
	// ============================================================================
	
	/**
	 * Actor的等级，等级可以影响施加的效果
	 *
	 * 作为所有施加出去的 GE 的 Level 参数（MakeOutgoingSpec 的第二个实参）使用：
	 * GE 里的 CurveTable、ModifierMagnitude 中按等级取值的曲线、以及 MMC 里读到的
	 * GetEffectLevel 都会受它影响。对只配了 SetByCaller 数值的 GE 则没有作用。
	 * ClampMin=1 只是编辑器输入限制，运行时不做钳制，代码/BP 里可以写入更小的值。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects", meta=(ClampMin="1"))
	float ActorLevel = 1.f;

	/**
	 * 进入区域（BeginOverlap）时施加的 Instant 效果（一次性，如瞬回血、瞬伤）
	 * 何时用：拾取即结算、不留下任何状态的效果（回血/加蓝/一次性加属性）。
	 * Instant 不产生 ActiveEffect，施加完立即结束，因此不会被句柄追踪；
	 * 每个目标每次进入都会生效一次（反复进出即可反复获得）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Enter")
	TArray<TSubclassOf<UGameplayEffect>> OnEnterInstantEffects;

	/**
	 * 进入区域（BeginOverlap）时施加的 Duration 效果（有时长，如 DoT、Hot）
	 * 何时用：进入毒池后定时掉血的场景。Duration GE 自带时长并与目标绑定，
	 * 本类不追踪它的句柄，所以离开区域不会自动停止——要让 DoT 提前结束时，
	 * 需改为 Infinite + bRemoveOnExit，或依赖 GE 自身的时长。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Enter")
	TArray<TSubclassOf<UGameplayEffect>> OnEnterDurationEffects;

	/**
	 * 进入区域（BeginOverlap）时施加的 Infinite 效果（永久持续，需手动移除）
	 * 何时用：光环区域、站在火上持续掉血、踩进减速场。
	 * 这类 GE 施加成功后会把句柄登记到 ActiveEffectsMap，由 bRemoveOnExit 负责移除；
	 * 若不设 bRemoveOnExit，效果会一直留在目标身上（本类没有其它清理入口）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Enter")
	TArray<TSubclassOf<UGameplayEffect>> OnEnterInfiniteEffects;

	/**
	 * 离开区域（EndOverlap）时施加的 Instant 效果（一次性，如瞬回血、瞬伤）
	 * 何时用：出门结算一次的场景，例如离开毒雾时补一次伤害、离开治疗区时补一次回复。
	 * 注意它只在"先进入过并触发了 OnEndOverlap"时才会执行；
	 * 若进入时 bDestroy 已销毁本 Actor，则永远不会触发。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Exit")
	TArray<TSubclassOf<UGameplayEffect>> OnExitInstantEffects;

	/**
	 * 离开区域（EndOverlap）时施加的 Duration 效果（有时长，如 DoT、Hot）
	 * 何时用："离开后仍被诅咒一段时间"这类离场惩罚。同样不追踪句柄，时长由 GE 自己决定。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Exit")
	TArray<TSubclassOf<UGameplayEffect>> OnExitDurationEffects;

	/**
	 * 离开区域（EndOverlap）时施加的 Infinite 效果（永久持续，需手动移除）
	 * 何时用：离开安全区后获得永久 Debuff。这些新效果同样会登记句柄，
	 * 但与刚被 bRemoveOnExit 清掉的那批不在同一次调用里，因此不会被误删。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Exit")
	TArray<TSubclassOf<UGameplayEffect>> OnExitInfiniteEffects;

	/**
	 * 离开区域时是否移除此前已追踪的 Infinite 效果
	 * 何时用：光环/毒气这类"离开就应停止"的区域必须打开；
	 * 只想给一次性增益、希望效果永久保留时保持关闭。
	 * 移除粒度由 bRemovalAllStacks / RemovalStackNumber 决定。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|Remove")
	bool bRemoveOnExit = false;

	/**
	 * 效果施加后是否销毁此 Actor（常用于一次性消耗品）
	 * OnBeginOverlap 与 OnEndOverlap 末尾都会检查它，语义是"本次交互结束后我使命完成"。
	 * 副作用：进入即销毁的拾取物不会再产生 OnEndOverlap，且它身上登记的
	 * Infinite 句柄会随 Actor 一起消失（目标身上残留的效果将无法再由本 Actor 移除）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	bool bDestroy = false;

	/**
	 * 移除效果时是否移除所有堆叠层数
	 * 打开后调用 RemoveActiveGameplayEffect(Handle, -1)，即整条效果连同全部层数一起清掉；
	 * 关闭时按 RemovalStackNumber 逐层剥离。只有配了 Stacking 的 GE 才会体现差别。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|Remove")
	bool bRemovalAllStacks = false;

	/**
	 * 移除效果时指定移除的堆叠层数（仅当 bRemovalAllStacks 为 false 时生效）
	 * 单位：层。默认 1 表示每次离开只剥一层。
	 * 注意：剥层后面本 Actor 会把句柄从 ActiveEffectsMap 中删掉，
	 * 若效果还剩层数，剩余层数就不再受本 Actor 管理（见 cpp 中 RemoveStoredEffects 的说明）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|Remove", meta=(ClampMin="1"))
	int32 RemovalStackNumber = 1;

private:
	// ============================================================================
	// 内部辅助
	// ============================================================================

	/** 批量施加效果数组：按数组顺序逐个调用 ApplyEffectToTarget，任一项为空/无效只跳过该项，不影响后续项 */
	void ApplyEffectArrayToTarget(AActor* InstigatorActor, AActor* TargetActor, const TArray<TSubclassOf<UGameplayEffect>>& EffectClasses);

	/** 移除目标 ASC 上所有已追踪的 Infinite 效果句柄 */
	void RemoveStoredEffects(UAbilitySystemComponent* TargetASC);

	// ============================================================================
	// 运行时数据
	// ============================================================================

	/**
	 * 已施加的 Infinite 效果句柄映射表
	 * Key:   目标 Actor 的 AbilitySystemComponent
	 * Value: 所有活跃句柄
	 *
	 * 生命周期/GC 注意：
	 *  - 该属性没有 Replicated 标记，也不 Transient；它只在施加效果的端（服务器）有意义，
	 *    客户端本来也拿不到有效的句柄。
	 *  - Key 是 UPROPERTY 强引用，会阻止对应 ASC 被回收。敌人死亡后若没走过
	 *    OnEndOverlap（例如尸体直接消失），那里的条目会一直留到本 Actor 销毁为止。
	 *  - 同一目标同一 GE 反复施加会向 Handles 里追加多个句柄（甚至指向同一个活跃效果实例），
	 *    移除时会逐个调用 RemoveActiveGameplayEffect。
	 */
	UPROPERTY()
	TMap<TObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandleArray> ActiveEffectsMap;
};
