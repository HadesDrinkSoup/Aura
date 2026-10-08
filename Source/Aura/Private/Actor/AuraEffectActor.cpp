// Study Game Ability System _Hades

#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

// ----------------------------------------------------------------------------
// Construction
// ----------------------------------------------------------------------------

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

// ----------------------------------------------------------------------------
// ApplyEffectToTarget
// ----------------------------------------------------------------------------

void AAuraEffectActor::ApplyEffectToTarget(AActor* InstigatorActor, AActor* TargetActor, const TSubclassOf<UGameplayEffect>& GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	if (!GameplayEffectClass) return;

	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	if (!InstigatorActor)
	{
		InstigatorActor = this;
	}
	EffectContextHandle.AddInstigator(InstigatorActor, this);

	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	if (!EffectSpecHandle.Data.Get() || !EffectSpecHandle.Data->Def.Get()) return;

	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	// Infinite 效果自动追踪句柄，供 bRemoveOnEnter / bRemoveOnExit 移除
	const bool bIsInfinite = EffectSpecHandle.Data->Def->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite)
	{
		ActiveEffectsMap.FindOrAdd(TargetASC).Handles.Add(ActiveEffectHandle);
	}
}

// ----------------------------------------------------------------------------
// ApplyEffectArrayToTarget (helper)
// ----------------------------------------------------------------------------

void AAuraEffectActor::ApplyEffectArrayToTarget(AActor* InstigatorActor, AActor* TargetActor, const TArray<TSubclassOf<UGameplayEffect>>& EffectClasses)
{
	for (const TSubclassOf<UGameplayEffect>& EffectClass : EffectClasses)
	{
		ApplyEffectToTarget(InstigatorActor, TargetActor, EffectClass);
	}
}

// ----------------------------------------------------------------------------
// RemoveStoredEffects (helper)
// ----------------------------------------------------------------------------

void AAuraEffectActor::RemoveStoredEffects(UAbilitySystemComponent* TargetASC)
{
	if (FActiveGameplayEffectHandleArray* ActiveEffectHandles = ActiveEffectsMap.Find(TargetASC))
	{
		const int32 StacksToRemove = bRemovalAllStacks ? -1 : RemovalStackNumber;
		for (const FActiveGameplayEffectHandle& Handle : ActiveEffectHandles->Handles)
		{
			TargetASC->RemoveActiveGameplayEffect(Handle, StacksToRemove);
		}
		ActiveEffectsMap.Remove(TargetASC);
	}
}

// ----------------------------------------------------------------------------
// OnBeginOverlap
// ----------------------------------------------------------------------------

void AAuraEffectActor::OnBeginOverlap(AActor* InstigatorActor, AActor* TargetActor)
{
	// 1. 再施加进入效果
	if (!OnEnterInstantEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnEnterInstantEffects);
	}
	if (!OnEnterDurationEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnEnterDurationEffects);
	}
	if (!OnEnterInfiniteEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnEnterInfiniteEffects);
	}

	// 2. 销毁自身（一次性消耗品模式）
	if (bDestroy)
	{
		Destroy();
	}
}

// ----------------------------------------------------------------------------
// OnEndOverlap
// ----------------------------------------------------------------------------

void AAuraEffectActor::OnEndOverlap(AActor* InstigatorActor, AActor* TargetActor)
{
	// 1. 先移除已追踪的旧效果（如进入时施加的毒气 DoT），再施加新效果（如永久 MaxHP-10），
	//    避免新 Infinite 效果被自己的 bRemoveOnExit 误删
	if (bRemoveOnExit)
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
		{
			RemoveStoredEffects(TargetASC);
		}
	}

	// 2. 再施加离开效果
	if (!OnExitInstantEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnExitInstantEffects);
	}
	if (!OnExitDurationEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnExitDurationEffects);
	}
	if (!OnExitInfiniteEffects.IsEmpty())
	{
		ApplyEffectArrayToTarget(InstigatorActor, TargetActor, OnExitInfiniteEffects);
	}

	// 3. 销毁自身（一次性消耗品模式）
	if (bDestroy)
	{
		Destroy();
	}
}