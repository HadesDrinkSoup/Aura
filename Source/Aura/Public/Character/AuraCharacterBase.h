// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "GameFramework/Character.h"
#include "Interface/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

// 前置声明：头文件里只用到指针/类引用，避免把实现细节扩散到所有包含本头文件的编译单元
class UAuraGameplayAbility;
struct FGameplayEffectContextHandle;
class UGameplayEffect;
class UAuraAttributeSet;
class UAuraAbilitySystemComponent;

/**
 * 角色基类：玩家角色（AAuraCharacter）与敌人（AAuraEnemy）的共同父类。
 *
 * 职责：
 *  - 实现 IAbilitySystemInterface，把 ASC 暴露给 GAS 运行时；
 *  - 实现 ICombatInterface，给出"武器尖端"的世界坐标，供近战判定/投射物起点使用；
 *  - 统一初始属性的施加流程（InitDefaultAttributes），并区分"职业默认配置"与"本 BP 个体覆盖"；
 *  - 统一默认技能的授予流程（AddCharacterAbilities）。
 *
 * 生命周期约定：先初始化 ASC（子类重写 InitActorInfo），再施加属性 GE、最后授予技能；
 * 这个顺序不能颠倒——属性 GE 中的 MMC 依赖 ASC 已 InitAbilityActorInfo，
 * 而技能被授予时可能立刻读取属性做初始化。
 *
 * 本类不直接创建 ASC/AttributeSet：玩家侧的它们归属于 AAuraPlayerState（为了跨死亡保留），
 * 敌人侧则在子类构造函数里直接创建，因此在基类里它们都可能是 nullptr。
 */
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	/** 构造函数：创建武器组件并挂到角色网格的手部插槽，同时配置胶囊体/网格体的碰撞响应 */
	AAuraCharacterBase();

	/** 返回 GAS 使用的 ASC；在 InitActorInfo 完成前可能为 nullptr，调用方需自行判空 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 近战判定/特效起点：武器骨骼网格上 WeaponTipSocketName 插槽的世界坐标 */
	virtual FVector GetCombatSocketLocation() override;
	
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;

	/** 职业的读写：BP 里配的是默认值，战斗关由 GameMode 在 Possess 之前按选人结果覆盖 */
	void SetCharacterClass(ECharacterClass InClass) { CharacterClass = InClass; }
	ECharacterClass GetCharacterClass() const { return CharacterClass; }
	
protected:
	/**
	 * 初始化 ASC 的 ActorInfo（子类实现）。
	 * 基类留空：玩家角色从 PlayerState 取 ASC/AttributeSet，敌人自己创建，二者时机与来源都不同。
	 */
	virtual void InitActorInfo() {};

	/**
	 * 施加初始属性（武器/职业 GE）。
	 * 仅在服务器执行，内部会按"职业配置表 → 本 BP 覆盖"的优先级解析出 Primary/Derived/Pool 三个 GE；
	 * 顺序固定为 Primary → Derived → AdditionalInitialEffects → Pool，原因见实现处注释。
	 */
	virtual void InitDefaultAttributes();

	// 帮助创建和应用GameplayEffect类中的游戏效果规范
	/**
	 * 为自身创建并施加一个 GameplayEffect。
	 * 同一套样板（MakeOutgoingSpec → 校验句柄 → ApplyGameplayEffectSpecToSelf）
	 * 被 Primary/Derived/Pool/个体差异四类 GE 复用，因此抽成静态函数。
	 * @param EffectClass 要施加的 GE 类，为空时直接返回
	 * @param ASC         目标 ASC，为空时直接返回
	 * @param EffectContext 由调用方一次性构建、供所有 GE 共享的上下文（含 Instigator/SourceObject）
	 * @param Level       GE 等级，默认 1；调用方按角色等级传入以便 CurveTable 取值
	 */
	static void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, UAbilitySystemComponent* ASC, const FGameplayEffectContextHandle& EffectContext, float Level = 1.f);
	
	/** 授予"职业默认技能 + 本 BP 附加技能"，仅服务器执行；由子类在 Possess/BeginPlay 时调用 */
	void AddCharacterAbilities() const;

	/** 取全局职业配置表中本角色职业的默认配置；没配职业信息时返回 nullptr */
	const FCharacterClassDefaultInfo* FindClassDefaultInfo() const;

	/**
	 * 起始技能总表：本 BP 额外配置的技能 + 职业默认技能 + 配置表的通用能力（CommonAbilities）。
	 * 三者取并集并用 AddUnique 去重，顺序为 BP → 职业 → 通用；
	 * 通用能力与职业无关，职业配置缺失时仍会下发。
	 */
	TArray<TSubclassOf<UAuraGameplayAbility>> GetCombinedStartupAbilities() const;
	
	/** 武器网格（手部武器），用于取攻击插槽坐标；不参与碰撞 */
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	/** 武器上作为"打击点/特效起点"的插槽名，必须与骨骼网格上的插槽一致 */
	UPROPERTY(EditAnywhere, Category="Combat")
	FName WeaponTipSocketName;
	
	/** GAS 组件：玩家侧来自 PlayerState，敌人侧在构造函数中创建；由子类在 InitActorInfo 中赋值 */
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	
	/** 属性集：与 ASC 同源（玩家侧来自 PlayerState，敌人侧自建），供 MMC/UI 读取属性定义 */
	UPROPERTY()
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;

	/**
	 * 职业：决定使用哪套属性 GE 与默认技能。
	 * 属性 GE 本身在 Project Settings → Game → Aura Game Settings 的 DA_CharacterClassInfo 里配置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class")
	ECharacterClass CharacterClass = ECharacterClass::Elementalist;

	/**
	 * 可选：直接在本 BP 上覆盖职业的属性 GE（留空则使用职业配置表）。
	 * 通常只在「职业表里没有的特殊个体」上使用。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> PoolAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TSubclassOf<UGameplayEffect> DerivedAttributes;

	/**
	 * 个体差异：在职业 GE 之后追加施加的 GE。
	 * 例如「精英哥布林」用一个 Multiply MaxHealth 的 GE，而不必复制整套职业 GE。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalInitialEffects;
	
private:
	/** 本 BP 额外配置的起始技能（在职业默认技能之前，最终取并集并去重） */
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UAuraGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;
};
