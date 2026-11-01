// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

class ACharacter;

/**
 * 属性标签 -> 属性访问器的委托类型（返回 FGameplayAttribute，不是属性值）。
 * 用途见 UAuraAttributeSet::TagsToAttributesMap：给 UI 提供「标签 ↔ 属性定义」的运行时映射，
 * UAttributesMenuController 遍历它给每个属性挂变更回调，再按标签回查属性当前值。
 */
DECLARE_DELEGATE_RetVal(FGameplayAttribute, FAttributeDelegate);

/**
 * 一次 GE 执行所涉及的「施法者 / 受击者」双方信息快照。
 *
 * 由 UAuraAttributeSet::SetEffectProperties 从 FGameplayEffectModCallbackData 里拆出来：
 * GE 回调只给出 EffectSpec 与目标 ASC，想知道"这一下是谁打的、打的是谁"必须顺着 EffectContext
 * 回溯原始 Instigator，再逐级取 ASC → AvatarActor → PlayerController / Character。
 * 拆成结构体的原因是 PostGameplayEffectExecute 后续逻辑（伤害/治疗/死亡、UI 通知）都需要这套信息，
 * 而多取一次就要多一次空值判断，集中在一处更好维护。
 *
 * 空值约定：每个字段都是"取得到才有值"，全部可能为 nullptr，使用前必须判空。典型来源：
 *  - 环境伤害 / 初始化属性 GE 没有真实施法者 → Instigator 侧整组为空；
 *  - 敌人（AI）ASC 没有 PlayerController → Controller 为空，但 Avatar/Character 有值；
 *  - 目标是道具/可破坏物等非 Character 的 Actor → Character 为空，但 ASC/Avatar 有值。
 */
USTRUCT(BlueprintType)
struct FEffectProperties
{
	GENERATED_BODY()
	/** 默认构造：所有成员按声明处的默认值（全部为 nullptr）初始化，不访问任何引擎对象 */
	FEffectProperties() {};

	/**
	 * 本次 GE 的执行上下文（Data.EffectSpec.GetContext() 的拷贝）。
	 * 施法者信息就是从它取出来的：GetOriginalInstigatorAbilitySystemComponent() 会沿上下文链
	 * 回溯到最初发起者（而非中间转发者）。上下文也可能无效（例如没有设置 Instigator 的系统性 GE）。
	 */
	UPROPERTY()
	FGameplayEffectContextHandle EffectContextHandle;
	/**
	 * 施法者的 ASC（原始 Instigator 的 ASC）。为 nullptr 时说明这次 GE 没有x真实施法者，
	 * 此时下面三个 Instigator 字段都保持为空，调用方不应把它当成"未知玩家"来处理。
	 */
	UPROPERTY()
	UAbilitySystemComponent* InstigatorASC = nullptr;
	/**
	 * 施法者 ASC 的 AvatarActor —— 实际在世界里表现的那个角色（玩家侧通常是 Character，
	 * 而不是持有 ASC 的 PlayerState）。只有 InstigatorASC 及其 AbilityActorInfo/AvatarActor 都有效时才赋值。
	 */
	UPROPERTY()
	AActor* InstigatorAvatarActor = nullptr;
	/**
	 * 施法者对应的 PlayerController：只有"施法者由玩家控制"时才有值，AI 施法者恒为 nullptr。
	 * 常用于客户端侧的特效/音效朝向、以及"只有玩家才能触发的成就/统计"判断。
	 */
	UPROPERTY()
	APlayerController* InstigatorController = nullptr;
	/**
	 * InstigatorAvatarActor 转成 ACharacter 的结果；Avatar 不是 Character（例如挂 ASC 的载具/道具）时为空。
	 * 需要角色网格、动画、移动组件时用它，不要直接强转 InstigatorAvatarActor。
	 */
	UPROPERTY()
	ACharacter* InstigatorCharacter = nullptr;

	/**
	 * 受击者（目标）的 ASC。注意：它不是 Data.Target 本身，
	 * 而是用 UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetAvatarActor) 通过 Avatar 反查到的，
	 * 只有 Data.Target 的 AbilityActorInfo 与 AvatarActor 都有效时才会赋值。
	 */
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;
	/**
	 * 受击者的 AvatarActor。对于玩家目标，这是 Character 而不是 PlayerState；
	 * 它是"属性实际挂在哪具身体上"的答案，扣血表现、受击特效都应对它做。
	 */
	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;
	/** 受击者的 PlayerController：目标不是玩家控制时为空（敌人被击中时恒为空） */
	UPROPERTY()
	APlayerController* TargetController = nullptr;
	/** TargetAvatarActor 的 ACharacter 视图；目标不是 Character 时为空 */
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

/**
 * Aura 的属性集：GAS 中所有 GameplayAttribute 的容器，也是属性钳制与"受击后处理"的落点。
 *
 * 持有者与复制：
 *  - 玩家侧由 AAuraPlayerState 创建（与 ASC 同源，跨死亡保留），敌人侧由 AAuraEnemy 创建；
 *  - 属性通过 GetLifetimeReplicatedProps 里的 DOREPLIFETIME_CONDITION_NOTIFY 全量复制
 *    （COND_None + REPNOTIFY_OnChanged：值变化才发，并在客户端触发对应 OnRep_*）；
 *  - 属性的权威写入只发生在服务器（GE 施加），客户端只接收复制结果，因此钳制逻辑
 *    （PreAttributeChange / PostGameplayEffectExecute）在客户端通常也会跑一遍但结果与服务器一致。
 *
 * 属性分三组（Category 与 UPROPERTY 声明一致）：
 *  - Pool Attributes     ：Health / Mana，会被钳制到 [0, 对应 Max]，是 UI 血条/蓝条直接读取的值；
 *  - Primary Attributes  ：Strength / Intelligence / Agility / Vigor，由 Primary GE 以加法写入的"根属性"；
 *  - Derived Attributes  ：由各职业 MMC 依据主属性 + 等级算出，通过 Infinite 派生 GE 写入，
 *    主属性变化时 GAS 自动重算，因此不能手动设置。
 *
 * 设计要点 / 注意：
 *  - 本类没有 Meta（如 Damage）属性：伤害 GE 直接改 Health，不存在"Meta 中转再扣血"的流程，
 *    因此 PostGameplayEffectExecute 里只有钳制，没有 Meta → Health 的结算；
 *  - Infinite 派生 GE 改写 MaxHealth/MaxMana 时不走 PostGameplayEffectExecute，
 *    Health/Mana 的重新钳制必须在 PostAttributeChange 里补（否则上限变小时血条会超上限）；
 *  - TagsToAttributesMap 在构造函数里静态绑定，供 UI 层按标签取属性定义与值。
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/**
	 * 构造函数：建立 TagsToAttributesMap（属性标签 → 对应属性的 GetXxxAttribute 访问器）。
	 * 只做静态绑定，不读取任何实例属性值，因此 CDO 构造时执行也安全。
	 * 这张表是 UI（UAttributesMenuController）驱动属性面板的唯一来源：新增属性时必须同步在这里登记，
	 * 否则 UI 面板里既不会显示该属性，也不会随它刷新。
	 */
	UAuraAttributeSet();

	/** 注册需要网络复制的属性列表 */
	/**
	 * 注册所有属性的复制规则与 OnRep 通知。
	 * 统一使用 COND_None（无条件参与复制）+ REPNOTIFY_OnChanged（客户端收到变化时才调用 OnRep_*），
	 * 而不是 COND_OwnerOnly：因为敌人血条等 UI 需要其他客户端也能读到属性值。
	 * 当前注册的是全部 21 个属性：Health/Mana、Strength/Intelligence/Agility/Vigor、
	 * MaxHealth/MaxMana/PhysicalAttack/MagicalAttack/Armor/MagicResistance/CriticalChance/
	 * HitChance/DodgeChance/AttackSpeed/CooldownReduction/HealthRegen/ManaRegen/MoveSpeed。
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 属性被修改「之前」的钳制点：GAS 在把新值提交给聚合器之前调用。
	 * 这里只钳制池属性 Health / Mana 到 [0, 当前 MaxXxx]。
	 *
	 * 注意两点：
	 *  - 本次修改的属性是 MaxHealth/MaxMana 时，GetMaxHealth() 读到的还是旧上限，
	 *    所以"上限变小后把当前值压回去"不能只靠这里，必须配合 PostAttributeChange；
	 *  - PreAttributeChange 只影响本次修改的最终值，不会改写 BaseValue，
	 *    直接 SetHealth() 之类的路径也可能绕过它，故 PostGameplayEffectExecute 里还有一层钳制。
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** 属性改变后回调：MaxHealth / MaxMana 变化时重新钳制 Health / Mana（Infinite 派生 GE 走这条路径） */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/**
	 * GE 执行完毕后的收尾：仅在服务器调用（客户端的属性变化来自复制，不会走这里）。
	 * 此时本次 GE 的所有 Modifier 都已结算、聚合器已完成，GetMaxHealth()/GetMaxMana() 是最终值，
	 * 所以这里是"最终钳制"的正确位置。
	 *
	 * 当前实现只做钳制与一次调试日志：Health/Mana 压回 [0, Max]，MaxHealth/MaxMana 变化后把池属性压回新上限。
	 * 本项目没有 Meta 属性，因此不存在"把 Meta Damage 结算进 Health"的流程。
	 * 注意：健康日志直接解引用 EffectProperties.TargetAvatarActor，未判空（见任务汇报中的疑似问题）。
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/**
	 * 属性标签 → 属性访问器（GetXxxAttribute）的映射，构造函数中一次性填好。
	 * Key 是 AuraGameplayTags 里的 Attributes_Pool_* / Attributes_Primary_* / Attributes_Derived_* 标签，
	 * Value 是返回 FGameplayAttribute 的静态方法委托。
	 * 目前消费者是 UAttributesMenuController：遍历它给每个属性挂 FOnAttributeChangeData 回调，
	 * 再通过 DA_AttributesInfo 里的同标签行显示名称/描述/当前数值。
	 */
	TMap<FGameplayTag, FAttributeDelegate> TagsToAttributesMap;

	// ============================================================================
	//  Pool Attributes（池属性）
	// ============================================================================

	/**
	 * 池属性 · 当前生命值。
	 * 服务器上的伤害/治疗 GE 直接改写它（没有 Meta 属性做中转），随后复制给客户端；
	 * 取值被钳制在 [0, MaxHealth]：PreAttributeChange 与 PostGameplayEffectExecute 各管一层，
	 * 上限变小时由 PostAttributeChange 兜底。UI 血条读的就是它。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Pool Attributes")
	FGameplayAttributeData Health;
	/** 生成 Get/Set/Init 与 FGameplayAttribute 访问器；SetHealth 走聚合器，会先命中 PreAttributeChange 的 [0, MaxHealth] 钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Health)

	/**
	 * 池属性 · 当前法力值。
	 * 由技能消耗/回复 GE 改写，复制规则与 Health 相同（COND_None + REPNOTIFY_OnChanged）。
	 * 钳制范围 [0, MaxMana]，钳制点与 Health 完全对称（Pre/PostAttributeChange + PostGameplayEffectExecute）。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Mana, BlueprintReadOnly, Category = "Pool Attributes")
	FGameplayAttributeData Mana;
	/** 生成访问器；SetMana 走聚合器，会先命中 PreAttributeChange 的 [0, MaxMana] 钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Mana)

	// ============================================================================
	//  Primary Attributes（重要属性）
	// ============================================================================

	/**
	 * 主属性 · 力量：影响物理攻击、最大生命、护甲（见 AuraGamepalyTags.h 中该标签的说明）。
	 * 由 Primary GE 以 Add 方式写入，本类不对它做钳制（理论上可被减益压到负值，
	 * MMC 读取时由 UAuraMMCBase::GetCapturedMagnitude 统一按 0 兜底）。
	 * 复制方式同其它属性：COND_None + REPNOTIFY_OnChanged。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Strength, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	/** 生成访问器；SetStrength 走聚合器，会触发依赖它的派生属性 MMC 重算 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Strength)

	/**
	 * 主属性 · 智力：影响魔法攻击、最大法力、魔法抗性、暴击率、冷却缩减、法力恢复。
	 * 由 Primary GE 加法写入，本类不钳制（MMC 侧按非负读取）。复制：COND_None + REPNOTIFY_OnChanged。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Intelligence, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	/** 生成访问器；SetIntelligence 走聚合器，会触发依赖它的派生属性 MMC 重算 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Intelligence)

	/**
	 * 主属性 · 敏捷：影响物理攻击、暴击率、命中、闪避、攻击速度、移动速度。
	 * 由 Primary GE 加法写入，本类不钳制（MMC 侧按非负读取）。复制：COND_None + REPNOTIFY_OnChanged。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Agility, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Agility;
	/** 生成访问器；SetAgility 走聚合器，会触发依赖它的派生属性 MMC 重算 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Agility)

	/**
	 * 主属性 · 活力：影响最大生命、最大法力、护甲、魔法抗性、生命恢复。
	 * 由 Primary GE 加法写入，本类不钳制（MMC 侧按非负读取）。复制：COND_None + REPNOTIFY_OnChanged。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Vigor, BlueprintReadOnly, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	/** 生成访问器；SetVigor 走聚合器，会触发依赖它的派生属性 MMC 重算 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Vigor)

	// ============================================================================
	//  Derived Attributes（派生属性）
	// ============================================================================

	/**
	 * 派生属性 · 最大生命值。由职业 MMC 计算后经 Infinite 派生 GE 写入，主属性变化时自动重算。
	 * 以元素法师 UMMC_Elementalist_MaxHealth 为例：65 + 活力 × 7 + 力量 × 2 + 等级 × 7，下限 1；
	 * 其它职业用各自的 MMC（同目录 Warrior/Ranger/Tank），系数不同。
	 * 本类不钳制它，但它在 PostAttributeChange / PostGameplayEffectExecute 里是 Health 的钳制上限。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MaxHealth;
	/** 生成访问器；由派生 GE 的 MMC 结果写入，不要用它手动改上限（会被聚合器覆盖） */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MaxHealth)

	/**
	 * 派生属性 · 最大法力值。元素法师 MMC：120 + 智力 × 15 + 活力 × 2 + 等级 × 9，下限 0；
	 * 与 MaxHealth 一样经 Infinite 派生 GE 写入，并作为 Mana 的钳制上限。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxMana, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MaxMana;
	/** 生成访问器；由派生 GE 的 MMC 结果写入 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MaxMana)

	/**
	 * 派生属性 · 物理攻击力。元素法师 MMC：3 + 力量 × 1 + 敏捷 × 0.3 + 等级 × 0.6，下限 0。
	 * 供物理伤害类 GE/技能读取，本类不钳制（下限由 MMC 保证）。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_PhysicalAttack, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData PhysicalAttack;
	/** 生成访问器；值与 MMC 结果一致，不参与池属性钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, PhysicalAttack)

	/**
	 * 派生属性 · 魔法攻击力。元素法师 MMC：12 + 智力 × 3.4 + 等级 × 1.5，下限 0。
	 * 供法术伤害类 GE/技能读取，本类不钳制。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MagicalAttack, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MagicalAttack;
	/** 生成访问器；值与 MMC 结果一致，不参与池属性钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MagicalAttack)

	/**
	 * 派生属性 · 护甲（减免物理伤害）。元素法师 MMC：0 + 活力 × 0.8 + 力量 × 0.2 + 等级 × 0.15，下限 0。
	 * 注意这是"护甲值"而非百分比减伤，减伤公式在伤害计算侧，本类不钳制。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Armor, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData Armor;
	/** 生成访问器；值与 MMC 结果一致，不参与池属性钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, Armor)

	/**
	 * 派生属性 · 魔法抗性（减免魔法伤害）。
	 * 元素法师 MMC：3 + 活力 × 0.45 + 智力 × 0.35 + 等级 × 0.4，下限 0。本类不钳制。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MagicResistance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MagicResistance;
	/** 生成访问器；值与 MMC 结果一致，不参与池属性钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MagicResistance)

	/**
	 * 派生属性 · 暴击几率（百分比，100 = 必暴）。
	 * 元素法师 MMC：4 + 智力 × 0.05 + 敏捷 × 0.04，钳制在 [0, 75]（上限由 MMC 保证，本类不重复钳制）。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CriticalChance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData CriticalChance;
	/** 生成访问器；值域由 MMC 钳制，UI 直接按百分比显示 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, CriticalChance)

	/**
	 * 派生属性 · 命中几率（百分比）。
	 * 元素法师 MMC：80 + 敏捷 × 0.06 + 智力 × 0.05 + 等级 × 0.1，钳制在 [0, 100]。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_HitChance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData HitChance;
	/** 生成访问器；百分比属性，值域由 MMC 钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, HitChance)

	/**
	 * 派生属性 · 闪避几率（百分比）。
	 * 元素法师 MMC：2 + 敏捷 × 0.08，钳制在 [0, 60]（刻意不让闪避堆到无敌）。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_DodgeChance, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData DodgeChance;
	/** 生成访问器；百分比属性，值域由 MMC 钳制 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, DodgeChance)

	/**
	 * 派生属性 · 攻击速度（以 100 为基准的倍率刻度，不是秒数）。
	 * 元素法师 MMC：95 + 敏捷 × 0.15 + 等级 × 0.1，钳制在 [50, 250]（下限防止慢到无法出手）。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_AttackSpeed, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData AttackSpeed;
	/** 生成访问器；值域由 MMC 钳制在 [50, 250] */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, AttackSpeed)

	/**
	 * 派生属性 · 冷却缩减（百分比）。
	 * 元素法师 MMC：3 + 智力 × 0.25 + 等级 × 0.1，钳制在 [0, 40]（上限 40% 防止冷却归零）。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CooldownReduction, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData 	CooldownReduction;
	/** 生成访问器；值域由 MMC 钳制在 [0, 40] */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, CooldownReduction)

	/**
	 * 派生属性 · 生命恢复（每秒回复量）。
	 * 元素法师 MMC：0.5 + 活力 × 0.08 + 等级 × 0.03，下限 0。
	 * 这里只是"恢复速率"数值，实际的周期回复由对应的 Infinite GE（Period）施加。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_HealthRegen, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData 	HealthRegen;
	/** 生成访问器；值与 MMC 结果一致，下限由 MMC 保证 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, HealthRegen)

	/**
	 * 派生属性 · 法力恢复（每秒回复量）。
	 * 元素法师 MMC：3 + 智力 × 0.3 + 等级 × 0.1，下限 0；实际回复同样由周期型 Infinite GE 施加。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_ManaRegen, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData 	ManaRegen;
	/** 生成访问器；值与 MMC 结果一致，下限由 MMC 保证 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, ManaRegen)

	/**
	 * 派生属性 · 移动速度（UE 单位/秒，供 CharacterMovement 读取）。
	 * 元素法师 MMC：380 + 敏捷 × 0.25，下限 0；本类不钳制上限。
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed, BlueprintReadOnly, Category = "Derived Attributes")
	FGameplayAttributeData MoveSpeed;
	/** 生成访问器；值与 MMC 结果一致 */
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, MoveSpeed)
	
	// ============================================================================
	//  Meta Attributes（元属性）
	// ============================================================================
	UPROPERTY(BlueprintReadOnly, Category="Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UAuraAttributeSet, IncomingDamage)

	// ============================================================================
	//  OnRep 网络复制回调
	// ============================================================================

	/** Health 属性网络复制回调，属性值变化时自动调用 */
	/**
	 * Health 复制回调（仅在"值真的变了"且客户端收到时触发，服务器不触发）。
	 * 实现只有 GAMEPLAYATTRIBUTE_REPNOTIFY：把复制来的新值写进聚合器并按 OldValue 广播
	 * FOnAttributeChangeData，UI 血条的数值刷新依赖于它。没有额外的业务逻辑（不在这里做死亡判定）。
	 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	/** Mana 属性网络复制回调，属性值变化时自动调用 */
	/**
	 * Mana 复制回调：同 OnRep_Health，仅把新值写入聚合器并广播变更委托（UI 蓝条据此刷新）。
	 * 本函数不做任何钳制——钳制发生在服务器写属性时，客户端只接收结果。
	 */
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	/**
	 * Strength 复制回调：客户端收到主属性变化时同步到聚合器并广播。
	 * 客户端的派生属性由服务器算好后单独复制下来，这里不需要（也不会）重算 MMC。
	 */
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	/** Intelligence 复制回调：写入聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

	/** Agility 复制回调：写入聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_Agility(const FGameplayAttributeData& OldAgility) const;

	/** Vigor 复制回调：写入聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

	/** MaxHealth 属性网络复制回调，属性值变化时自动调用 */
	/**
	 * MaxHealth 复制回调：写聚合器 + 广播变更委托（UI 的"当前/上限"显示依赖它）。
	 * 注意它不会在客户端重新钳制 Health：服务器已经在 PostAttributeChange / PostGameplayEffectExecute
	 * 里把 Health 压到新上限，随后 Health 自身的复制会把结果带到客户端。
	 */
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	/** MaxMana 属性网络复制回调，属性值变化时自动调用 */
	/**
	 * MaxMana 复制回调：写聚合器 + 广播变更委托。
	 * 与 MaxHealth 一样，不在客户端重做 Mana 的钳制，依赖服务器先钳制再复制。
	 */
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	/** PhysicalAttack 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_PhysicalAttack(const FGameplayAttributeData& OldPhysicalAttack) const;

	/** MagicalAttack 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_MagicalAttack(const FGameplayAttributeData& OldMagicalAttack) const;

	/** Armor 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

	/** MagicResistance 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_MagicResistance(const FGameplayAttributeData& OldMagicResistance) const;

	/** CriticalChance 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_CriticalChance(const FGameplayAttributeData& OldCriticalChance) const;

	/** HitChance 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_HitChance(const FGameplayAttributeData& OldHitChance) const;

	/** DodgeChance 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_DodgeChance(const FGameplayAttributeData& OldDodgeChance) const;

	/** AttackSpeed 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed) const;

	/** CooldownReduction 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction) const;

	/** HealthRegen 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const;

	/** ManaRegen 复制回调：写聚合器 + 广播变更委托，无额外逻辑 */
	UFUNCTION()
	void OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen) const;

	/** MoveSpeed 复制回调：写聚合器 + 广播变更委托；当前没有把新速度应用回 CharacterMovement 的逻辑 */
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed) const;

private:
	/**
	 * 从 GE 的 ModCallbackData 解析出「施法者 / 受击者」双方快照，填入传入的结构体。
	 *
	 * 为什么需要它：FGameplayEffectModCallbackData 只给 EffectSpec（含 EffectContext）与目标 ASC，
	 * 而"谁造成的这次效果"必须走 EffectContext 的原始 Instigator ASC，
	 * 再顺着 ASC 的 AbilityActorInfo 取 AvatarActor / PlayerController，逐级都有空的可能。
	 *
	 * 取值路径：
	 *  - Instigator：EffectContext.GetOriginalInstigatorAbilitySystemComponent() → AbilityActorInfo
	 *    → AvatarActor / PlayerController → Cast<ACharacter>；
	 *  - Target：Data.Target.AbilityActorInfo → AvatarActor → Cast<ACharacter> +
	 *    UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AvatarActor) 反查 ASC。
	 * 任何一级无效时对应字段保持 nullptr，不做兜底猜测；结构体成员先由默认构造置空。
	 * 静态函数：不需要访问本 AttributeSet 的实例状态，纯粹是纯函数式的解析。
	 * @param Data             GE 执行回调数据（只读）
	 * @param EffectProperties 输出参数，调用方负责先默认构造
	 */
	static void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties);
};
