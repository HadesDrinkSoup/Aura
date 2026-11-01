// Study Game Ability System _Hades

#include "AbilitySystem/AuraAttributeSet.h"

#include "AuraGamepalyTags.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

/**
 * @file AuraAttributeSet.cpp
 * @brief 属性集实现：标签→属性映射表、属性复制注册、三层钳制（Pre/PostAttributeChange 与
 *        PostGameplayEffectExecute）、复制回调，以及从 GE 回调数据里解析施法者/受击者信息。
 *
 * 要点：
 *  - 属性写入的权威端是服务器：GE 在服务器结算后再复制到客户端，钳制逻辑因此也以服务器结果为准；
 *  - 钳制分三处，缺一不可：PreAttributeChange 管"写入前"，PostAttributeChange 管"Infinite 派生 GE
 *    直接改上限"这条不经过 PostGameplayEffectExecute 的路径，PostGameplayEffectExecute 管"Instant GE 结算后"；
 *  - 本项目没有 Meta（Damage）属性：伤害 GE 直接修改 Health，没有"Meta 中转"这一层；
 *  - 每个 OnRep_* 都只有 GAMEPLAYATTRIBUTE_REPNOTIFY 一行，属于"把复制值交给聚合器并广播给 UI"的样板代码。
 */

/**
 * 构造函数：建立 TagsToAttributesMap（属性标签 → 对应属性的静态访问器）。
 *
 * 为什么用 BindStatic 而不是 BindUObject：GetXxxAttribute 是 ATTRIBUTE_ACCESSORS_BASIC 生成的
 * 静态函数，不需要对象实例即可调用；这样委托不持有 this，CDO 构造期间也安全，
 * 而 UI 侧执行委托时再用 Execute() 得到的 FGameplayAttribute 去 AttributeSet 上取值。
 *
 * 三类属性都要登记：池属性（Health/Mana）、主属性（Strength/Intelligence/Agility/Vigor）、
 * 派生属性（MaxHealth…MoveSpeed）。漏登记不会报错，但 UAttributesMenuController 的属性面板
 * 会既列不出该属性、也不会随它刷新——这是本表唯一的消费者。
 *
 * 注意：构造里只登记映射，不写任何初始值。属性的初始数值全部由 Primary/Derived/Pool GE 提供；
 * 若某个角色没有被施加属性 GE，Health 与 MaxHealth 都会是 0，此时 PreAttributeChange 会把
 * 任何写入直接钳成 0，表现为"血条永远空"。
 */
UAuraAttributeSet::UAuraAttributeSet()
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	/** 池属性：UI 直接显示的两个数值，对应 Attributes_Pool_* 标签 */

	FAttributeDelegate HealthDelegate;
	HealthDelegate.BindStatic(GetHealthAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Pool_Health, HealthDelegate);

	FAttributeDelegate ManaDelegate;
	ManaDelegate.BindStatic(GetManaAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Pool_Mana, ManaDelegate);

	/** 重要属性：主属性由 Primary GE 加法写入，是派生属性的输入 */

	FAttributeDelegate StrengthDelegate;
	StrengthDelegate.BindStatic(GetStrengthAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Strength, StrengthDelegate);

	FAttributeDelegate IntelligenceDelegate;
	IntelligenceDelegate.BindStatic(GetIntelligenceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Intelligence, IntelligenceDelegate);

	FAttributeDelegate AgilityDelegate;
	AgilityDelegate.BindStatic(GetAgilityAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Agility, AgilityDelegate);

	FAttributeDelegate VigorDelegate;
	VigorDelegate.BindStatic(GetVigorAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Primary_Vigor, VigorDelegate);

	/** 派生属性：数值来自各职业 MMC（Infinite 派生 GE 写入），主属性变化时自动重算 */
	FAttributeDelegate MaxHealthDelegate;
	MaxHealthDelegate.BindStatic(GetMaxHealthAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MaxHealth, MaxHealthDelegate);

	FAttributeDelegate MaxManaDelegate;
	MaxManaDelegate.BindStatic(GetMaxManaAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MaxMana, MaxManaDelegate);

	FAttributeDelegate PhysicalAttackDelegate;
	PhysicalAttackDelegate.BindStatic(GetPhysicalAttackAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_PhysicalAttack, PhysicalAttackDelegate);

	FAttributeDelegate MagicalAttackDelegate;
	MagicalAttackDelegate.BindStatic(GetMagicalAttackAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MagicalAttack, MagicalAttackDelegate);

	FAttributeDelegate ArmorDelegate;
	ArmorDelegate.BindStatic(GetArmorAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_Armor, ArmorDelegate);

	FAttributeDelegate MagicResistanceDelegate;
	MagicResistanceDelegate.BindStatic(GetMagicResistanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MagicResistance, MagicResistanceDelegate);

	FAttributeDelegate CriticalChanceDelegate;
	CriticalChanceDelegate.BindStatic(GetCriticalChanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_CriticalChance, CriticalChanceDelegate);

	FAttributeDelegate HitChanceDelegate;
	HitChanceDelegate.BindStatic(GetHitChanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_HitChance, HitChanceDelegate);

	FAttributeDelegate DodgeChanceDelegate;
	DodgeChanceDelegate.BindStatic(GetDodgeChanceAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_DodgeChance, DodgeChanceDelegate);

	FAttributeDelegate AttackSpeedDelegate;
	AttackSpeedDelegate.BindStatic(GetAttackSpeedAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_AttackSpeed, AttackSpeedDelegate);

	FAttributeDelegate CooldownReductionDelegate;
	CooldownReductionDelegate.BindStatic(GetCooldownReductionAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_CooldownReduction, CooldownReductionDelegate);

	FAttributeDelegate HealthRegenDelegate;
	HealthRegenDelegate.BindStatic(GetHealthRegenAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_HealthRegen, HealthRegenDelegate);

	FAttributeDelegate ManaRegenDelegate;
	ManaRegenDelegate.BindStatic(GetManaRegenAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_ManaRegen, ManaRegenDelegate);

	FAttributeDelegate MoveSpeedDelegate;
	MoveSpeedDelegate.BindStatic(GetMoveSpeedAttribute);
	TagsToAttributesMap.Add(GameplayTags.Attributes_Derived_MoveSpeed, MoveSpeedDelegate);
}

/**
 * 注册所有属性的复制规则与复制通知回调。
 *
 * 每个属性都用 DOREPLIFETIME_CONDITION_NOTIFY(类, 属性, COND_None, REPNOTIFY_OnChanged)：
 *  - COND_None          —— 不附加额外条件，属性进入复制列表（而不是"从不复制"）；
 *  - REPNOTIFY_OnChanged —— 只有服务器上该值真的变化时才发，并在客户端调用对应的 OnRep_*；
 *    用 REPNOTIFY_Always 会在每次属性复制的网络包都回调，造成无意义的 UI 刷新。
 *
 * 为什么不用 COND_OwnerOnly：敌人的血条由 (UEnemyHealthBarWidgetController) 在本地显示，
 * 需要非拥有者侧的客户端也能读到属性值，所以池属性与派生属性对所有客户端可见；
 * 代价是带宽，本项目属性数量有限，可以接受。
 * 为什么不用 COND_SimulatedOnly：玩家角色的 ASC 在 PlayerState 上，属性本来就需要在拥有者客户端
 * 与模拟端都保持一致。
 *
 * 每个 UPROPERTY(ReplicatedUsing = OnRep_Xxx) 都必须在这里有一条对应的注册，漏掉会导致：
 * 属性在服务器变化但客户端收不到，且 OnRep 永远不会被调用（编译期不报错，只有运行时才发现）。
 */
void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_OnChanged);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Agility, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_OnChanged);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalAttack, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MagicalAttack, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MagicResistance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalChance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HitChance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, DodgeChance, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, AttackSpeed, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CooldownReduction, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegen, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MoveSpeed, COND_None, REPNOTIFY_OnChanged);
}

/**
 * 属性写入前的钳制：把池属性限制在 [0, 当前上限]。
 *
 * 调用时机：GAS 在属性即将被修改时调用（GE 施加、SetNumericAttributeBase/SetHealth 等写入路径都会走到），
 * 传入的 NewValue 是"即将生效的值"，改它就能改写本次结果。
 *
 * 关键限制（为什么还需要另外两处钳制）：
 *  - 在这里改 NewValue 只影响"这一次修改的最终值"，不会写回 BaseValue；
 *    需要钳制 BaseValue 时要用 PreAttributeBaseChange（本类未重写）；
 *  - 修改 MaxHealth/MaxMana 时，此处 GetMaxHealth() 读到的仍是旧上限，
 *    所以"上限变小、当前血超过上限"必须在 PostAttributeChange 里补一次；
 *  - 只有 Health/Mana 被钳制，主属性/派生属性不在这里限制（百分比类属性的上限在各自 MMC 里钳制）。
 *
 * 上下界都用 GetMaxHealth()/GetMaxMana()：属性未初始化时为 0，此时任何写入都会被压成 0。
 */
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 血量不允许为负（死亡下限）也不允许超过上限；上限的"变小"场景由 PostAttributeChange 兜底
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}

	// 法力同理：技能消耗不能把蓝条压成负数，回复不能超过上限
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

/**
 * 属性改变后的回调：上限变化时把池属性重新钳制回新上限。
 *
 * 为什么必须有这一层：派生属性 GE 是 Infinite 型的，它对 MaxHealth/MaxMana 的修改由聚合器直接生效，
 * 这类 GE 永远不执行 PostGameplayEffectExecute（只有 Instant GE 才会），
 * 因此"上限被调低导致当前血量 > 上限"只能在这里修。
 * 这里用 SetHealth/SetMana 写回，会再次进入 PreAttributeChange，形成一次自洽的钳制；
 * 因为 NewValue 已经是新上限，重入不会造成无限递归（第二次 Clamp 结果相同）。
 */
void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue, const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 派生属性 GE 是 Infinite：MaxHealth / MaxMana 由聚合器直接改写，
	// 不会走 PostGameplayEffectExecute，因此在这里补齐 Health / Mana 的钳制。
	if (Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, NewValue));
	}
	if (Attribute == GetMaxManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, NewValue));
	}
}

/**
 * GE 执行完毕后的收尾（只在服务器、且只对 Instant GE 调用）。
 *
 * 时序：一次 Instant GE 的所有 Modifier 已经结算完、聚合器已刷新，
 * 所以这里 GetMaxHealth()/GetMaxMana() 一定是最终值——这正是"最终钳制"该放的位置。
 * Duration/Infinite GE 不会走到这里（它们的属性修改是持续的，靠 Pre/PostAttributeChange 约束）。
 *
 * 四个分支分别处理：
 *  1) Health 变化：钳到 [0, MaxHealth]（伤害/治疗的最终结果可能越界）；
 *  2) Mana 变化 ：钳到 [0, MaxMana]；
 *  3) MaxHealth 变化：把当前 Health 压到新上限以下（例如"减最大生命的诅咒"结算后）；
 *  4) MaxMana 变化 ：同理把 Mana 压到新上限以下。
 *
 * 关于 Meta 属性：本项目没有 Damage 之类的 Meta 属性，也就没有"Meta 结算 → 扣 Health → 清零 Meta"
 * 这一段，伤害 GE 是直接对 Health 做加法（负数即扣血），所以这里只有钳制。
 *
 * 注意：Health 分支里的 UE_LOG 直接解引用 EffectProperties.TargetAvatarActor，
 * 没有判空——若目标 ASC 没有有效的 AvatarActor，这一行会崩。此处只描述现状，未改动行为。
 */
void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 先把施法者/受击者信息解析出来，后续死亡判定、受击表现、UI 通知都要用它
	FEffectProperties EffectProperties;
	SetEffectProperties(Data, EffectProperties);

	// 在 GE 执行完毕后对属性进行钳制
	// 这是正确钳制位置 —— 此时所有属性修改都已生效，
	// GetMaxHealth() / GetMaxMana() 返回的都是最终值
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		UE_LOG(LogTemp, Warning, TEXT("%s 初始生命值：%f"), *EffectProperties.TargetAvatarActor->GetName(), GetHealth());
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	// 当 MaxHealth 降低时，重新钳制 Health 防止 CurrentHealth > MaxHealth
	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetMaxManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	
	// 当造成伤害时，读出 Meta 值造成伤害并钳制
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (LocalIncomingDamage > 0.f)
		{
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			UE_LOG(LogTemp, Warning, TEXT("%s 生命值受到伤害 %f， 当前生命值%f"), *EffectProperties.TargetAvatarActor->GetName(), LocalIncomingDamage, GetHealth());
			//是否致命
			const bool bFatal = NewHealth <= 0.f;
			if (!bFatal)
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
				EffectProperties.TargetASC->TryActivateAbilitiesByTag(TagContainer); 
			}
		}
	}
}

// ----------------------------------------------------------------------------
// 属性复制回调（OnRep）
//
// 当属性在服务器端发生变化、同步到客户端时调用。
// GAMEPLAYATTRIBUTE_REP NOTIFY 宏会自动处理：
//   1. 将新值同步到 AbilitySystemComponent
//   2. 触发属性变更委托，通知所有监听者（UI、GameplayEffect 等）
//
// 补充：这些函数只在客户端因复制而触发，服务器上属性变化不会走 OnRep（服务器是权威端，直接改值）；
// 每个函数体都只有一行宏调用，没有任何额外的业务逻辑——不在这里做死亡判定、不在这里钳制、
// 也不在这里重算派生属性（派生值由服务器算好后单独复制）。
// ----------------------------------------------------------------------------

/** Health 由服务器变化并复制到客户端：把复制值写入聚合器并按 OldHealth 广播变更，UI 血条依赖这次广播 */
void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

/** Mana 复制回调：写聚合器 + 广播变更委托；此处不做钳制（服务器已钳制后再复制） */
void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

/** Strength 复制回调：写聚合器 + 广播；客户端不据此重算 MMC，派生属性由服务器算好后单独复制 */
void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

/** Intelligence 复制回调：写聚合器 + 广播；不重算 MMC */
void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

/** Agility 复制回调：写聚合器 + 广播；不重算 MMC */
void UAuraAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Agility, OldAgility);
}

/** Vigor 复制回调：写聚合器 + 广播；不重算 MMC */
void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

/** MaxHealth 复制回调：写聚合器 + 广播（UI 的"当前/上限"依赖它）；不在客户端重钳 Health */
void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

/** MaxMana 复制回调：写聚合器 + 广播；不在客户端重钳 Mana */
void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

/** PhysicalAttack 复制回调：写聚合器 + 广播；纯展示/计算用数值，无额外逻辑 */
void UAuraAttributeSet::OnRep_PhysicalAttack(const FGameplayAttributeData& OldPhysicalAttack) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalAttack, OldPhysicalAttack);
}

/** MagicalAttack 复制回调：写聚合器 + 广播；无额外逻辑 */
void UAuraAttributeSet::OnRep_MagicalAttack(const FGameplayAttributeData& OldMagicalAttack) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MagicalAttack, OldMagicalAttack);
}

/** Armor 复制回调：写聚合器 + 广播；无额外逻辑 */
void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

/** MagicResistance 复制回调：写聚合器 + 广播；无额外逻辑 */
void UAuraAttributeSet::OnRep_MagicResistance(const FGameplayAttributeData& OldMagicResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MagicResistance, OldMagicResistance);
}

/** CriticalChance 复制回调：写聚合器 + 广播；值域 [0,75] 由 MMC 保证，这里不重复钳制 */
void UAuraAttributeSet::OnRep_CriticalChance(const FGameplayAttributeData& OldCriticalChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalChance, OldCriticalChance);
}

/** HitChance 复制回调：写聚合器 + 广播；值域 [0,100] 由 MMC 保证 */
void UAuraAttributeSet::OnRep_HitChance(const FGameplayAttributeData& OldHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HitChance, OldHitChance);
}

/** DodgeChance 复制回调：写聚合器 + 广播；值域 [0,60] 由 MMC 保证 */
void UAuraAttributeSet::OnRep_DodgeChance(const FGameplayAttributeData& OldDodgeChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, DodgeChance, OldDodgeChance);
}

/** AttackSpeed 复制回调：写聚合器 + 广播；值域 [50,250] 由 MMC 保证 */
void UAuraAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, AttackSpeed, OldAttackSpeed);
}

/** CooldownReduction 复制回调：写聚合器 + 广播；值域 [0,40] 由 MMC 保证 */
void UAuraAttributeSet::OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CooldownReduction, OldCooldownReduction);
}

/** HealthRegen 复制回调：写聚合器 + 广播；这里只是恢复速率数值，周期回复由 Infinite GE 施加 */
void UAuraAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegen, OldHealthRegen);
}

/** ManaRegen 复制回调：写聚合器 + 广播；周期回复同样由 Infinite GE 施加 */
void UAuraAttributeSet::OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegen, OldManaRegen);
}

/** MoveSpeed 复制回调：写聚合器 + 广播；当前没有把新速度写回 CharacterMovement 的逻辑 */
void UAuraAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MoveSpeed, OldMoveSpeed);
}

/**
 * 从 GE 的 ModCallbackData 解析「施法者 / 受击者」双方信息。
 *
 * 为什么需要自己拆：GE 回调只提供 EffectSpec（含 EffectContext）与目标 ASC，
 * "这次效果是谁造成的"必须顺着 EffectContext 取原始施法者的 ASC，再沿 ASC 的 AbilityActorInfo
 * 取 AvatarActor / PlayerController——每一级都可能为空，所以必须逐级判空而不是强转。
 *
 * 为什么用 GetOriginalInstigatorAbilitySystemComponent 而不是 GetInstigator：
 * 后者返回的是"最近一次设置"的 AActor，且被嵌套的 GE（如由 GE 触发的 GE）会把它覆盖成中间对象；
 * "Original" 版本沿上下文链回溯到最初施加者，且直接用 ASC 指针，省掉一次 Actor→ASC 的查找。
 * 之所以从 AbilityActorInfo 取 PlayerController 而不是 Cast<APlayerController>(ASC->GetOwner())：
 * 玩家侧 ASC 挂在 PlayerState 上，它的 Owner 是 PlayerState 而不是 Controller。
 *
 * 目标侧同理：先看 Data.Target 的 AbilityActorInfo 是否有效，再从 AvatarActor 反查 ASC
 * （UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent 会沿 Avatar/Owner 链找 IAbilitySystemInterface），
 * 这样拿到的 ASC 与 Avatar 是同一实体，避免把"属性所在对象"和"表现对象"搞混。
 *
 * 空值后果（本函数不做兜底赋值，全部保持默认构造的 nullptr）：
 *  - AI 敌人施法：InstigatorController 为空，但 InstigatorASC/AvatarActor/Character 有值；
 *  - 初始化属性 GE（AAuraCharacterBase::InitDefaultAttributes 设置 Instigator=Character）：
 *    InstigatorController 有值（玩家）或为空（敌人），Avatar 是角色本身；
 *  - 目标是道具/可破坏物：TargetCharacter 为空。
 * 调用方（PostGameplayEffectExecute）必须按需判空；当前实现里的日志没有判空（仅描述现状）。
 *
 * @param Data             GE 执行回调数据（只读，不修改）
 * @param EffectProperties 输出参数，调用方负责先默认构造（成员默认均为 nullptr）
 */
void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties)
{
	// --- 获取事件源（Instigator）信息 ---
	EffectProperties.EffectContextHandle = Data.EffectSpec.GetContext();
	// 原始施法者的 ASC：为 nullptr 说明这次 GE 没有设置过 Instigator（例如某些系统性/环境 GE）
	EffectProperties.InstigatorASC = EffectProperties.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (EffectProperties.InstigatorASC)
	{
		// 双重判空：ASC 存在不代表它完成过 InitAbilityActorInfo（未初始化的 ASC 拿不到 Avatar）
		if (EffectProperties.InstigatorASC->AbilityActorInfo.IsValid() && EffectProperties.InstigatorASC->AbilityActorInfo->AvatarActor.IsValid())
		{
			EffectProperties.InstigatorAvatarActor = EffectProperties.InstigatorASC->AbilityActorInfo->AvatarActor.Get();
			// Avatar 不一定是 Character（可能是挂 ASC 的道具），转换失败时保持 nullptr
			EffectProperties.InstigatorCharacter = Cast<ACharacter>(EffectProperties.InstigatorAvatarActor);

			// 只有玩家控制的 ASC 才有 PlayerController（AI 施法者为空）
			if (EffectProperties.InstigatorASC->AbilityActorInfo->PlayerController.IsValid())
			{
				EffectProperties.InstigatorController = EffectProperties.InstigatorASC->AbilityActorInfo->PlayerController.Get();
			}
		}
	}

	// --- 获取目标（Target）信息 ---
	// 目标 ASC 由 Data.Target 给出，但"目标是谁"仍要经 AvatarActor 解析：
	// 玩家侧的 ASC 属于 PlayerState，属性所在对象并不是玩家看到的那具角色
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		EffectProperties.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		EffectProperties.TargetCharacter = Cast<ACharacter>(EffectProperties.TargetAvatarActor);
		EffectProperties.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(EffectProperties.TargetAvatarActor);

		// 目标不是玩家（敌人/道具）时为空
		if (Data.Target.AbilityActorInfo->PlayerController.IsValid())
		{
			EffectProperties.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		}
	}
}
