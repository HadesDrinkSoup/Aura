// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
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
 * 基类这两个成员只是「缓存指针」（*）：真正的所有者是 PlayerState 或敌人自身，
 * 子类在 InitActorInfo 里把所有权者的实例填进来，基类只借它们实现接口与授予技能。
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
	virtual FVector GetCombatSocketLocation() const override;
	
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	
	/** 死亡逻辑 */
	virtual void Death_Implementation() override;

protected:
	/**
	 * 初始化 ASC 的 ActorInfo（子类实现）。
	 * 基类留空：玩家角色从 PlayerState 取 ASC/AttributeSet，敌人自己创建，二者时机与来源都不同。
	 */
	virtual void InitActorInfo() {};

	/**
	 * 施加初始属性（武器/职业 GE）。
	 * 仅在服务器执行，内部会按"职业配置表 → 本 BP 覆盖"的优先级解析出 Primary/Derived/Pool 三个 GE；
	 * 灵属（ElementAffinity）走两层：职业表上的通用灵属 GE + 本 BP 的个体差异列表，
	 * 顺序固定为 Primary → Derived → 通用灵属 → 灵属个体差异 → AdditionalInitialEffects → Pool，
	 * 原因见实现处注释。
	 */
	virtual void InitDefaultAttributes();

	// 帮助创建和应用GameplayEffect类中的游戏效果规范
	/**
	 * 为自身创建并施加一个 GameplayEffect。
	 * 同一套样板（MakeOutgoingSpec → 校验句柄 → ApplyGameplayEffectSpecToSelf）
	 * 被 Primary/Derived/通用灵属/灵属个体/Pool/个体差异六类 GE 复用，因此抽成静态函数。
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
	
	/** 死亡溶解：把网格体与武器各自换成自己的溶解材质 */
	void Dissolve();
	
	/** 溶解时间线（蓝图实现）。参数是「网格体」的动态材质实例。*/
	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void StartWeaponDissolveTimeLine(UMaterialInstanceDynamic* DynamicMaterialInstance);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void StartMeshDissolveTimeLine(UMaterialInstanceDynamic* DynamicMaterialInstance);
	
	
	/** 武器网格（手部武器），用于取攻击插槽坐标；不参与碰撞 */
	UPROPERTY(EditAnywhere, Category = "Aura|Character|Combat", meta = (DisplayName = "武器网格"))
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	/** 武器上作为"打击点/特效起点"的插槽名，必须与骨骼网格上的插槽一致 */
	UPROPERTY(EditAnywhere, Category = "Aura|Character|Combat", meta = (DisplayName = "武器顶端插槽"))
	FName WeaponTipSocketName;
	
	/**
	 * 缓存的 GAS 组件指针：玩家侧来自 PlayerState，敌人侧在构造函数中创建；由子类在 InitActorInfo 中赋值。
	 * 本类不拥有它，所有权在 AuraPlayerState / AAuraEnemy，因此这里只是引用。
	 */
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AbilitySystemComponent;
	
	/**
	 * 缓存的属性集指针：与 ASC 同源（玩家侧来自 PlayerState，敌人侧自建），供 MMC/UI 读取属性定义。
	 * 同样不拥有所有权，仅作缓存。
	 */
	UPROPERTY()
	TObjectPtr<UAuraAttributeSet> AttributeSet;

	/**
	 * 职业：决定使用哪套属性 GE 与默认技能。
	 * 属性 GE 本身在 GameMode 上配置的 DA_CharacterClassInfo 里（见 AAuraGameModeBase）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aura|Character|Class", meta = (DisplayName = "角色职业"))
	ECharacterClass CharacterClass = ECharacterClass::Elementalist;

	/**
	 * 可选：直接在本 BP 上覆盖职业的属性 GE（留空则使用职业配置表）。
	 * 通常只在「职业表里没有的特殊个体」上使用。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aura|Character|Attributes", meta = (DisplayName = "池属性GE"))
	TSubclassOf<UGameplayEffect> PoolAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aura|Character|Attributes", meta = (DisplayName = "主属性GE"))
	TSubclassOf<UGameplayEffect> PrimaryAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aura|Character|Attributes", meta = (DisplayName = "派生属性GE"))
	TSubclassOf<UGameplayEffect> DerivedAttributes;
	
	//个体差异：在职业 GE 之后追加施加的 GE。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aura|Character|Attributes", meta = (DisplayName = "个体差异GE"))
	TSubclassOf<UGameplayEffect> AdditionalEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aura|Character|Combat", meta = (DisplayName = "角色溶解材质"))
	TObjectPtr<UMaterialInstance> MeshDissolveMaterialInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aura|Character|Combat", meta = (DisplayName = "武器溶解材质"))
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;
	
private:
	/** 将死亡复制到所有客户端 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandleDeath();
	
	/** 武器跟随网格体溶解的重复计时器句柄 */
	FTimerHandle DissolveSyncTimerHandle;
	
	/** 本 BP 额外配置的起始技能（在职业默认技能之前，最终取并集并去重） */
	UPROPERTY(EditAnywhere, Category = "Aura|Character|Abilities", meta = (DisplayName = "初始技能"))
	TArray<TSubclassOf<UAuraGameplayAbility>> StartupAbilities;
	
	/** 攻击命中动画蒙太奇 */
	UPROPERTY(EditAnywhere, Category = "Aura|Character|Combat", meta = (DisplayName = "受击动画"))
	TObjectPtr<UAnimMontage> HitReactMontage;
};
