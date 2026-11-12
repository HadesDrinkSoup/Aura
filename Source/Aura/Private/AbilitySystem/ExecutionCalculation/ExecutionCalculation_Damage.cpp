// Study Game Ability System _Hades

/**
 * @file ExecutionCalculation_Damage.cpp
 * @brief 伤害结算：技能基础伤害 + 施法者攻击力 → 目标防御减伤 → 写入 IncomingDamage。
 *
 * 数据流（谁提供什么）：
 *  - 技能（UAuraProjectileSpell::SpawnProjectile）在 Spec 上用 SetByCaller 写入两样东西：
 *    攻击类型 tag（Damage.Physical / Damage.Magical / Damage.Magical.Fire …）作为 key，技能基础伤害作为 magnitude。
 *    同一个 Spec 可以同时写物理与魔法，构成混合伤害；
 *  - 本类负责按 tag 把基础伤害分流：物理标签吃 PhysicalAttack，魔法标签吃 MagicalAttack，
 *    各自算成原始伤害，再按受击者的护甲/魔抗分别减伤后相加；
 *  - 结果只写入 IncomingDamage（Meta 属性），扣血、飘字、受击、死亡动画全部交给
 *    UAuraAttributeSet::PostGameplayEffectExecute，这里不直接改 Health；
 *  - 防御减伤是纯函数：CalculateFinalDamage 是本类的成员函数，本类直接调用。
 *
 * 为什么用 Meta 属性中转，而不是直接对 Health 做 Additive：
 *  - 伤害需要先算清楚、再一次性结算：写 Health 时想再叠暴击/固伤/护盾就没地方挂了；
 *  - PostGameplayEffectExecute 里能同时拿到「伤害数值」和「施法者/受击者」，天然是飘字与死亡判定的落点；
 *  - Meta 属性每次结算后被清零（见 AttributeSet），不会被误当成持续数值。
 *
 * 扩展点（本类刻意不做，避免概率逻辑混进数值计算）：
 *  - 命中/闪避：施法前比对 HitChance 与 DodgeChance，判定不过就不施加 GE，伤害天然为 0；
 *  - 暴击：在写入 IncomingDamage 之前乘以 CriticalChance 判定的暴击倍率即可，本类输出的就是未暴击伤害。
 */

#include "AbilitySystem/ExecutionCalculation/ExecutionCalculation_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraGamepalyTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Actor.h"

const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DamageStatics;
	return DamageStatics;
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
}

void UExecutionCalculation_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	
	if (!SourceASC || !TargetASC) return;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	// 按伤害类型标签把「技能基础伤害」分成物理/魔法两段：
	// 物理标签吃 PhysicalAttack 加成，魔法标签吃 MagicalAttack 加成，混合伤害两段各自结算。
	// 直接遍历 Spec 上的 SetByCaller 表，而不是遍历固定的 tag 列表：以后新增伤害子标签
	// （Damage.Magical.Ice 之类）这里不用改；MatchesTag 是父级匹配，Fire 会归到 Magical。
	float PhysicalBase = 0.f;
	float MagicalBase = 0.f;
	for (const TPair<FGameplayTag, float>& Pair : Spec.SetByCallerTagMagnitudes)
	{
		const float BaseDamage = FMath::Max(Pair.Value, 0.f);
		if (BaseDamage <= 0.f) continue;
		
		if (Pair.Key.MatchesTag(FAuraGameplayTags::Get().Bonus_Physical))
		{
			PhysicalBase += BaseDamage;
		}
		if (Pair.Key.MatchesTag(FAuraGameplayTags::Get().Bonus_Magical))
		{
			MagicalBase += BaseDamage;
		}
	}

	const bool bHasPhysical = PhysicalBase > 0.f;
	const bool bHasMagical = MagicalBase > 0.f;
	// 一个伤害类型的 base 都没写：这张 GE 不是伤害，直接不输出
	if (!bHasPhysical && !bHasMagical) return;

	// 判断是否命中
	float HitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().HitChanceDef, EvaluateParameters, HitChance);
	float DodgeChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DodgeChanceDef, EvaluateParameters, DodgeChance);
	const float FinalHitChance = FMath::Clamp(HitChance - DodgeChance, 0.f, 100.f);
	// 用 [0,100) 的浮点骰子：100% 命中必定命中，0% 命中必定未命中
	const bool bIsDodge = FMath::FRandRange(0.f, 100.f) >= FinalHitChance;
	UAuraAbilitySystemLibrary::SetIsDodge(EffectContextHandle, bIsDodge);
	if (bIsDodge)
	{
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

	
	// 最终伤害
	float FinalDamage = 0.f;
	if (bHasPhysical)
	{
		FinalDamage += CalculateFinalDamage(RawPhysical, Armor);
	}
	if (bHasMagical)
	{
		FinalDamage += CalculateFinalDamage(RawMagical, MagicResistance);
	}
	
	// 暴击：在本类掷一次骰子，掷中就乘暴击倍率。
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalChanceDef, EvaluateParameters, CriticalChance);
	CriticalChance = FMath::Max(CriticalChance, 0.f);
	const bool bIsCriticalHit = FMath::RandRange(0.f, 100.f) < CriticalChance;
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bIsCriticalHit);
	if (bIsCriticalHit)
	{
		FinalDamage *= CriticalAttackConstant;
	}
	
	// 输出IncomingDamage（Meta属性）
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}

float UExecutionCalculation_Damage::CalculateFinalDamage(const float Damage, const float Defense) const
{
	if (Damage <= 0.f) return 0.f;
	const float FinalDamage = Damage * (1.f -  Defense / (Defense + DefenseConstant));
	return FinalDamage;
}
