// Study Game Ability System _Hades

#pragma once

#include "GameplayEffectTypes.h"
#include "AuraGameplayEffectContext.generated.h"

/**
 * 自定义 GE 上下文：在引擎自带的 Instigator / EffectCauser / HitResult 之外，
 * 额外携带「这一下是否被闪避」「这一下是否暴击」两个布尔标记。
 *
 * 为什么需要它：暴击/闪避是在伤害结算（ExecutionCalculation_Damage）里掷骰得出的，
 * 但真正要用到这两个信息的消费方（飘字的字号/颜色、受击表现、后端的战斗日志）
 * 可能不在同一次函数调用里，甚至不在同一个端上（GE 会随 ActiveGameplayEffect 复制到客户端）。
 * 把结果挂在 EffectContext 上，等于挂在「这一次效果」上，谁拿到 Spec 谁就能读。
 *
 * 两个标记是普通 bool（1 字节），UPROPERTY 保证它们能被结构体资产的属性系统识别；
 * 但网络上走的是本类手写的 NetSerialize（见 .cpp），不是属性自动序列化。
 */
USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
public:
	bool IsMiss() const {return bIsMiss;}
	bool IsCriticalHit() const {return bIsCriticalHit;}

	void SetIsMiss(const bool bInIsMiss) {bIsMiss = bInIsMiss;}
	void SetIsCriticalHit(const bool bInIsCriticalHit) {bIsCriticalHit = bInIsCriticalHit;}

	/**
	 * 必须返回派生类型本身，且必须覆盖的是 GetScriptStruct()（基类里虚函数的真名）。
	 *
	 * 这个返回值有三处用途，任何一处返回基类都会让两个 bool 静默失效：
	 *  1. FGameplayEffectContextHandle::NetSerialize 先写/读 struct 名，
	 *     客户端据此决定 new 哪种上下文实例（写基类名，客户端就只会 new 出基类，读不到我们的 bit）；
	 *  2. 同一个函数里用 GetScriptStruct()->GetCppStructOps()->NetSerialize 派发，
	 *     struct 名不对就一直调基类的 NetSerialize；
	 *  3. FGameplayEffectContextDeleter 用它取 DestroyStruct 来释放内存。
	 */
	virtual UScriptStruct* GetScriptStruct() const override {return StaticStruct();}

	/**
	 * 复制上下文：基类实现里 new 的是 FGameplayEffectContext，派生字段会被静默截断。
	 * GE Spec 复制、SetContext、快照等路径都会走这里，所以必须自己 new 派生类型并整份拷贝。
	 */
	virtual FGameplayEffectContext* Duplicate() const override
	{
		FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// HitResult 是 TSharedPtr，按基类的做法深拷贝一份，避免复制体与本体共享同一个命中结果
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	UPROPERTY()
	bool bIsMiss = false;

	UPROPERTY()
	bool bIsCriticalHit = false;
};

/**
 * 结构体操作特征：没有这个特化，FAuraGameplayEffectContext 会被当成「没有原生 NetSerialize」的普通结构体。
 *
 * 具体后果：FGameplayEffectContextHandle::NetSerialize 里会走
 *   if (ScriptStruct->StructFlags & STRUCT_NetSerializeNative) { ... } else { ABILITY_LOG(Fatal, ...) }
 * 也就是直接 Fatal 崩掉 —— 因为它的注释写得很清楚：没有原生 NetSerialize 的上下文无法走属性级序列化。
 * 基类的特化不会继承给派生结构体，所以派生类型必须自己写一份。
 */
template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,	// 启用本类的 NetSerialize（同时点亮 STRUCT_NetSerializeNative）
		WithCopy = true				// 让 CopyScriptStruct 走 C++ 拷贝赋值而不是逐属性拷贝；本类非 POD，默认值本就是 true，写出来是为了与基类写法一致
	};
};
