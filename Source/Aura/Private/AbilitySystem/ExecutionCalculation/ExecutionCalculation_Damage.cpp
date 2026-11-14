// Study Game Ability System _Hades

/**
 * @file ExecutionCalculation_Damage.cpp
 * @brief 伤害结算：技能基础伤害 + 施法者攻击力 → 目标防御减伤 → 元素修正 → 暴击 → 写入 IncomingDamage。
 *
 * 数据流（谁提供什么）：
 *  - 技能（UAuraProjectileSpell::SpawnProjectile）把 DamageTypes 里的每一项都以 SetByCaller 写进 Spec：
 *    key 是伤害加成类型 tag，magnitude 是按技能等级查表得到的值；
 *    同一个技能还会把自己的元素（DamageType）写进效果上下文，随 Spec 一起走到结算端。
 *  - 本类负责按 tag 把基础伤害分流成两条通道：
 *      Bonus.Physical → 加上 PhysicalAttack，之后按受击者 Armor 减伤；
 *      Bonus.Magical  → 加上 MagicalAttack，之后按受击者 MagicResistance 减伤；
 *    通道决定「怎么算」，元素决定「算什么」：技能标了 Damage.Element.Fire，这一击的两条通道
 *    就整体按火元素结算，不需要再额外配一条元素伤害；
 *  - 元素修正是本类的核心（受击者五项亲和都参与，不只是「克制它的那一项」）：
 *      先减掉通用基线：有效亲和 = max(亲和 - AffinityBaseline, 0)，通用 +10 不直接换伤害
 *      出手增伤 = 施法者[本次元素]有效亲和 × (+ Gain)
 *      受击减伤 = 受击者[克制本次元素]有效亲和 × (- Mitigation)
 *      受击增伤 = 受击者[被本次元素克制，即生它]有效亲和 × (+ Weakness)
 *      三段都用同一种递减公式，常数分别是 ElementalMasteryConstant / ElementalResistanceConstant / ElementalWeaknessConstant。
 *    元素唯一来源是效果上下文里的 DamageType，没有标就不是元素攻击（不修正）。
 *  - 结果只写入 IncomingDamage（Meta 属性），扣血、飘字、受击、死亡动画全部交给
 *    UAuraAttributeSet::PostGameplayEffectExecute，这里不直接改 Health。
 *  - 减伤是纯函数：CalculateFinalDamage / CalculateElementalMasteryRatio / CalculateElementElementResistancesRatio / CalculateElementalWeaknessRatio。
 *
 * 为什么用 Meta 属性中转，而不是直接对 Health 做 Additive：
 *  - 伤害需要先算清楚、再一次性结算：直接改 Health 时想再叠暴击/固伤/护盾就没地方挂了；
 *  - PostGameplayEffectExecute 里能同时拿到「伤害数值」和「施法者/受击者」，天然是飘字与死亡判定的落点；
 *  - Meta 属性每次结算后被清零（在 AttributeSet），不会被误当成持续数值。
 *
 * 调试日志：
 *  - 控制台变量 Aura.Damage.Log（默认 1）开启时，每次结算都把各段基础值/攻击/防御、
 *    元素修正（增伤、减伤、弱点各自的比例与最终倍率）、暴击与最终伤害打到日志
 *    （统一 [AuraDamage] 前缀）；调参时在输出日志里过滤 [AuraDamage] 即可对比不同亲和配置；
 *  - 命中/闪避与暴击的骰子也在本类掷：闪避写 0 伤害，AttributeSet 照常走受击流程，
 *    暴击在写入 IncomingDamage 前乘 CriticalAttackConstant。
 */

#include "AbilitySystem/ExecutionCalculation/ExecutionCalculation_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraGamepalyTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"


static AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DamageStatics;
	return DamageStatics;
}

/**
 * 用「已经注册好的」native GameplayTag 重建两张标签映射表。
 *
 * 为什么需要这一步：AuraDamageStatics 是函数内 static，第一次调用发生在
 * UExecutionCalculation_Damage 的 CDO 构造里（模块加载期），而 native GameplayTag 要等到
 * UAuraAssetManager::StartInitialLoading → FAuraGameplayTags::InitializeNativeGameplayTags 才注册。
 * 构造期 Add 进去的 key 全是无效 tag，且同名 key 会相互覆盖，最终表里只剩一条空 key：
 *   Statics.TagsToAttributesCaptureDefs.Find(Attributes.Affinity.Water)  → nullptr
 *   Statics.ElementToSourceAffinityDefs.Find(Damage.Element.Fire)        → nullptr
 * 于是 Execute 里读不到任何亲和值，增伤/减伤/弱点恒为 0，元素系数恒为 1，元素系统静默失效。
 *
 * 现在把「建表」收进本函数：构造函数末尾调用一次，首次 Execute 再调用一次兜底。
 * 若构造时 tag 已经注册就当场建好；否则保持未建状态，等真正结算时重建，之后 bTagMapsBuilt 直接短路。
 */
void AuraDamageStatics::EnsureTagMapsBuilt()
{
	if (bTagMapsBuilt) return;

	const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
	// 防御：万一某次结算发生在 tag 注册之前，就不要建出一张空表并把它标记为已建，
	// 保持 bTagMapsBuilt = false，下一次结算再重试。
	if (!Tags.Attributes_Affinity_Wood.IsValid() || !Tags.Damage_Element_Fire.IsValid())
	{
		return;
	}

	TagsToAttributesCaptureDefs.Reset();
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Derived_Armor, ArmorDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Derived_MagicResistance, MagicResistanceDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Derived_HitChance, HitChanceDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Derived_DodgeChance, DodgeChanceDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Derived_CriticalChance, CriticalChanceDef);

	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Affinity_Wood, WoodDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Affinity_Earth, EarthDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Affinity_Lightning, LightningDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Affinity_Water, WaterDef);
	TagsToAttributesCaptureDefs.Add(Tags.Attributes_Affinity_Fire, FireDef);

	ElementToSourceAffinityDefs.Reset();
	ElementToSourceAffinityDefs.Add(Tags.Damage_Element_Wood, SourceWoodDef);
	ElementToSourceAffinityDefs.Add(Tags.Damage_Element_Earth, SourceEarthDef);
	ElementToSourceAffinityDefs.Add(Tags.Damage_Element_Lightning, SourceLightningDef);
	ElementToSourceAffinityDefs.Add(Tags.Damage_Element_Water, SourceWaterDef);
	ElementToSourceAffinityDefs.Add(Tags.Damage_Element_Fire, SourceFireDef);

	bTagMapsBuilt = true;
}

UExecutionCalculation_Damage::UExecutionCalculation_Damage()
{
	// 登记捕获属性：不登记就无法在 Execute 里读到值（GetCapturedAttributeMagnitude 会直接失败）
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalAttackDef);
	RelevantAttributesToCapture.Add(DamageStatics().MagicalAttackDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().HitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().MagicResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().DodgeChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().WoodDef);
	RelevantAttributesToCapture.Add(DamageStatics().EarthDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningDef);
	RelevantAttributesToCapture.Add(DamageStatics().WaterDef);
	RelevantAttributesToCapture.Add(DamageStatics().FireDef);

	// 出手侧元素亲和力（施法者）必须单独登记，否则 GetCapturedAttributeMagnitude 读不到，
	RelevantAttributesToCapture.Add(DamageStatics().SourceWoodDef);
	RelevantAttributesToCapture.Add(DamageStatics().SourceEarthDef);
	RelevantAttributesToCapture.Add(DamageStatics().SourceLightningDef);
	RelevantAttributesToCapture.Add(DamageStatics().SourceWaterDef);
	RelevantAttributesToCapture.Add(DamageStatics().SourceFireDef);
}

void UExecutionCalculation_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	if (!SourceASC || !TargetASC) return;
	
	const AActor* SourceActor = SourceASC->GetOwnerActor();
	const AActor* TargetActor = TargetASC->GetOwnerActor();
	// 注意存成 FString：GetName() 返回临时对象，直接取 *GetName() 会留下悬垂指针
	const FString SourceName = SourceActor ? SourceActor->GetName() : FString(TEXT("None"));
	const FString TargetName = TargetActor ? TargetActor->GetName() : FString(TEXT("None"));

	// 本结构体是在 UExecutionCalculation_Damage 的 CDO 构造期创建的，而 CDO 在 Aura 模块加载时
	// 由 ProcessNewlyLoadedUObjects 构造，早于 UAuraAssetManager::StartInitialLoading 里的
	// FAuraGameplayTags::InitializeNativeGameplayTags，那时 map 的 key 全是空标签。
	// 元素修正靠这两张表把「元素/抗性标签」映射到「亲和力属性捕获定义」，key 空了就永远查不到，
	// 表现为：技能标了 Damage.Element.* 也吃不到任何元素增减伤。
	// 所以这里在第一次真正结算时补建一次（此时 native tags 必然已注册），之后直接短路。
	AuraDamageStatics& Statics = DamageStatics();
	Statics.EnsureTagMapsBuilt();

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	// ---------------------------------------------------------------------
	// 1. 收集「技能基础伤害」：物理 / 魔法两段
	// ---------------------------------------------------------------------
	// WarnIfNotFound = false：非本类型的伤害标签本来就不会出现在 Spec 上，
	// 用默认的 true 会让每次结算在日志里刷一堆 "SetByCaller tag not found"。
	float PhysicalBase = 0.f;
	float MagicalBase = 0.f;
	PhysicalBase += Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Bonus_Physical, false);
	MagicalBase += Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Bonus_Magical, false);

	const bool bHasPhysical = PhysicalBase > 0.f;
	const bool bHasMagical = MagicalBase > 0.f;

	// 物理/魔法一段都没写：这个 GE 不是伤害，直接不输出
	if (!bHasPhysical && !bHasMagical) return;

	// ---------------------------------------------------------------------
	// 2. 确定「这一击是什么元素」
	// ---------------------------------------------------------------------
	// 唯一来源：能力写进效果上下文的 DamageType（如 Damage.Element.Fire），
	// 由 UAuraProjectileSpell::SpawnProjectile 从能力默认值里带过来。
	// 不是元素 → 不做元素修正（倍率 = 1），纯物理/魔法技能不受影响。
	const FGameplayTag ContextDamageType = UAuraAbilitySystemLibrary::GetDamageType(EffectContextHandle);
	const bool bContextIsElemental = ContextDamageType.IsValid() && FAuraGameplayTags::Get().DamageTypesToResistances.Contains(ContextDamageType);

	const FGameplayTag AttackElementTag = bContextIsElemental ? ContextDamageType : FGameplayTag();

	// 判断是否命中
	float HitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().HitChanceDef, EvaluateParameters, HitChance);
	float DodgeChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DodgeChanceDef, EvaluateParameters, DodgeChance);
	const float FinalHitChance = FMath::Clamp(HitChance - DodgeChance, 0.f, 100.f);
	// 用 [0,100) 的浮点骰子：100% 命中必定命中，0% 命中必定未命中
	const float HitRoll = FMath::FRandRange(0.f, 100.f);
	const bool bIsDodge = HitRoll >= FinalHitChance;
	UAuraAbilitySystemLibrary::SetIsDodge(EffectContextHandle, bIsDodge);
	if (bIsDodge)
	{
		// 闪避照样输出 0 伤害：让 AttributeSet 能正常走受击 / 飘字流程
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, 0.f));
		return;
	}
	
	// 施法者攻击力加成：只取与伤害类型对应的那一项，另一项保持 0，避免物理技能吃到魔法攻击
	float PhysicalAttack = 0.f;
	float MagicalAttack = 0.f;
	if (bHasPhysical)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PhysicalAttackDef, EvaluateParameters, PhysicalAttack);
		PhysicalAttack = FMath::Max(PhysicalAttack, 0.f);
	}
	if (bHasMagical)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicalAttackDef, EvaluateParameters, MagicalAttack);
		MagicalAttack = FMath::Max(MagicalAttack, 0.f);
	}
	
	// 原始伤害，未计算防御
	const float RawPhysical = PhysicalBase + PhysicalAttack * AttackScaling;
	const float RawMagical  = MagicalBase + MagicalAttack * AttackScaling;
	
	// 受击者防御减伤
	float Armor = 0.f;
	float MagicResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, Armor);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicResistanceDef, EvaluateParameters, MagicResistance);
	Armor = FMath::Max(Armor, 0.f);
	MagicResistance = FMath::Max(MagicResistance, 0.f);
	
	// 各段合计：物理/魔法段已经各自吃过护甲/魔抗，元素修正下一步统一乘
	float FinalDamage = 0.f;
	if (bHasPhysical)
	{
		const float PhysicalFinal = CalculateFinalDamage(RawPhysical, Armor);
		FinalDamage += PhysicalFinal;
	}
	if (bHasMagical)
	{
		const float MagicalFinal = CalculateFinalDamage(RawMagical, MagicResistance);
		FinalDamage += MagicalFinal;
	}

	// =====================================================================
	// 元素修正
	// =====================================================================
	/**
	 * ElementalMasteryStrength / ElementalResistanceStrength / ElementalWeaknessStrength 调节权重，
	 * 没有元素时这些值保持 0 / 1，等价于「不做元素修正」。
	 */

	if (AttackElementTag.IsValid())
	{
		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();

		// 通过本次攻击的元素反查攻击者的元素亲和力用于增伤
		const FGameplayTag AttackElementAffinityTag = Tags.ElementAffinityPairs.FindRef(AttackElementTag);

		// 查找攻击元素对应的抵抗元素
		const FGameplayTag ResistanceElementTag = Tags.DamageTypesToResistances.FindRef(AttackElementTag);

		// 捕获攻击者的元素亲和力用于增伤
		float SourceElementAffinity = 0.f;
		if (const FGameplayEffectAttributeCaptureDefinition* GainDef = Statics.ElementToSourceAffinityDefs.Find(AttackElementTag))
		{
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(*GainDef, EvaluateParameters, SourceElementAffinity);
		}
		SourceElementAffinity = FMath::Max(SourceElementAffinity, 0.f);
		
		// 根据攻击发起者的元素精通计算增伤比例
		float MasteryRatio = CalculateElementMasteryRatio(SourceElementAffinity);
		
		// 元素抗性
		float ElementResistancesRatio = 0.f;
		// 元素弱点
		float ElementWeaknessRatio = 0.f;
		// 受击侧：五项元素亲和逐一判定
		for (const TPair<FGameplayTag, FGameplayTag>& Pair : Tags.ElementAffinityPairs)
		{
			const FGameplayTag& AffinityTag = Pair.Value;

			// 说明这项元素与本次伤害没有关系，直接跳过，连属性都不用读
			if (AffinityTag != ResistanceElementTag && !AttackElementAffinityTag.IsValid() && AttackElementAffinityTag != Tags.DamageTypesToResistances.FindRef(Pair.Key))
			{
				continue;
			}

			const FGameplayEffectAttributeCaptureDefinition* AffinityDef = Statics.TagsToAttributesCaptureDefs.Find(AffinityTag);
			if (!AffinityDef) continue;

			// 读受击者这一项亲和的当前值
			float AffinityValue = 0.f;
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(*AffinityDef, EvaluateParameters, AffinityValue);
			AffinityValue = FMath::Max(AffinityValue, 0.f);
			
			if (AffinityTag == ResistanceElementTag)
			{
				// 抗性：该项亲和越高 → 减伤比例高（水亲和高的人挨火球更不疼
				ElementResistancesRatio += CalculateElementResistancesRatio(AffinityValue);
			}
			else if (AttackElementAffinityTag == Tags.DamageTypesToResistances.FindRef(Pair.Key))
			{
				// 弱点：该项亲和高 → 增伤比例高（属木者被火克、木又生火，所以怕火）
				ElementWeaknessRatio += CalculateElementWeaknessRatio(AffinityValue);
			}
		}

		// 1 + 掌握增伤 - 克制减伤 + 弱点增伤 == 元素伤害系数
		float ElementCoefficient = FMath::Clamp(1.f + MasteryRatio - ElementResistancesRatio + ElementWeaknessRatio, ElementalMultiplierMin, ElementalMultiplierMax);

		FinalDamage *= ElementCoefficient;
	}
	// 暴击：在本类掷一次骰子，掷中就乘暴击倍率
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalChanceDef, EvaluateParameters, CriticalChance);
	CriticalChance = FMath::Max(CriticalChance, 0.f);
	const float CriticalRoll = FMath::FRandRange(0.f, 100.f);
	const bool bIsCriticalHit = CriticalRoll < CriticalChance;
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bIsCriticalHit);
	if (bIsCriticalHit)
	{
		FinalDamage *= CriticalAttackConstant;
	}
	
	// 输出 IncomingDamage（Meta 属性）
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}

float UExecutionCalculation_Damage::CalculateFinalDamage(const float Damage, const float Defense) const
{
	if (Damage <= 0.f) return 0.f;
	const float FinalDamage = Damage * (1.f -  Defense / (Defense + DefenseConstant));
	return FinalDamage;
}

// 元素出手增伤比例：有效亲和 = max(亲和 - 基准, 0)，通用 +10 只算基线，不再白送伤害。
float UExecutionCalculation_Damage::CalculateElementMasteryRatio(const float Affinity) const
{
	const float EffectiveAffinity = FMath::Max(Affinity - AffinityBaseline, 0.f);
	if (EffectiveAffinity <= 0.f) return 0.f;
	// 不要在这里再乘「攻击力」：攻击力已经在物理/魔法段算过一次，重复乘等于二次放大
	const float Ratio = FMath::Clamp(EffectiveAffinity / (EffectiveAffinity + ElementalMasteryConstant), 0.f, 1.f);
	return Ratio * ElementalMasteryStrength;
}

// 元素抵抗减伤比例：受击者「克制本元素」的亲和高于基准越多，挨这种元素越不疼。
float UExecutionCalculation_Damage::CalculateElementResistancesRatio(const float Affinity) const
{
	const float EffectiveAffinity = FMath::Max(Affinity - AffinityBaseline, 0.f);
	if (EffectiveAffinity <= 0.f) return 0.f;
	const float Ratio = FMath::Clamp(EffectiveAffinity / (EffectiveAffinity + ElementalResistanceConstant), 0.f, 1.f);
	return Ratio * ElementalResistanceStrength;
}

// 元素弱点增伤比例：受击者「被本元素克制」的亲和高于基准越多，挨这种元素越疼。
float UExecutionCalculation_Damage::CalculateElementWeaknessRatio(const float Affinity) const
{
	const float EffectiveAffinity = FMath::Max(Affinity - AffinityBaseline, 0.f);
	if (EffectiveAffinity <= 0.f) return 0.f;
	const float Ratio = FMath::Clamp(EffectiveAffinity / (EffectiveAffinity + ElementalWeaknessConstant), 0.f, 1.f);
	return Ratio * ElementalWeaknessStrength;
}
