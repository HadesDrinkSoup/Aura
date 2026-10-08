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
 */
USTRUCT()
struct FActiveGameplayEffectHandleArray
{
	GENERATED_BODY()

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
 * @note Infinite 类型的效果施加后会自动追踪其句柄；bRemoveOnEnter/bRemoveOnExit
 *       控制何时移除已追踪的效果
 */
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()

public:
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
	 */
	UFUNCTION(BlueprintCallable, Category="Applied Effects")
	void ApplyEffectToTarget(AActor* InstigatorActor, AActor* TargetActor, const TSubclassOf<UGameplayEffect>& GameplayEffectClass);

	/**
	 * Overlap 开始时的回调
	 * 依次执行：施加 OnEnterInstantEffects → OnEnterDurationEffects → OnEnterInfiniteEffects
	 */
	UFUNCTION(BlueprintCallable, Category="Applied Effects")
	void OnBeginOverlap(AActor* InstigatorActor, AActor* TargetActor);

	/**
	 * Overlap 结束时的回调
	 * 依次执行：若 bRemoveOnExit 则移除已追踪效果 → 施加 OnExitInstantEffects → OnExitDurationEffects → OnExitInfiniteEffects → 若 bDestroy 则销毁自身
	 */
	UFUNCTION(BlueprintCallable, Category="Applied Effects")
	void OnEndOverlap(AActor* InstigatorActor, AActor* TargetActor);

	// ============================================================================
	// 配置属性
	// ============================================================================
	
	/** Actor的等级，等级可以影响施加的效果 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects", meta=(ClampMin="1"))
	float ActorLevel = 1.f;

	/** 进入区域（BeginOverlap）时施加的 Instant 效果（一次性，如瞬回血、瞬伤） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Enter")
	TArray<TSubclassOf<UGameplayEffect>> OnEnterInstantEffects;

	/** 进入区域（BeginOverlap）时施加的 Duration 效果（有时长，如 DoT、Hot） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Enter")
	TArray<TSubclassOf<UGameplayEffect>> OnEnterDurationEffects;

	/** 进入区域（BeginOverlap）时施加的 Infinite 效果（永久持续，需手动移除） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Enter")
	TArray<TSubclassOf<UGameplayEffect>> OnEnterInfiniteEffects;

	/** 离开区域（EndOverlap）时施加的 Instant 效果（一次性，如瞬回血、瞬伤） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Exit")
	TArray<TSubclassOf<UGameplayEffect>> OnExitInstantEffects;

	/** 离开区域（EndOverlap）时施加的 Duration 效果（有时长，如 DoT、Hot） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Exit")
	TArray<TSubclassOf<UGameplayEffect>> OnExitDurationEffects;

	/** 离开区域（EndOverlap）时施加的 Infinite 效果（永久持续，需手动移除） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|On Exit")
	TArray<TSubclassOf<UGameplayEffect>> OnExitInfiniteEffects;

	/** 离开区域时是否移除此前已追踪的 Infinite 效果 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|Remove")
	bool bRemoveOnExit = false;

	/** 效果施加后是否销毁此 Actor（常用于一次性消耗品） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	bool bDestroy = false;

	/** 移除效果时是否移除所有堆叠层数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|Remove")
	bool bRemovalAllStacks = false;

	/** 移除效果时指定移除的堆叠层数（仅当 bRemovalAllStacks 为 false 时生效） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects|Remove", meta=(ClampMin="1"))
	int32 RemovalStackNumber = 1;

private:
	// ============================================================================
	// 内部辅助
	// ============================================================================

	/** 批量施加效果数组 */
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
	 */
	UPROPERTY()
	TMap<TObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandleArray> ActiveEffectsMap;
};
