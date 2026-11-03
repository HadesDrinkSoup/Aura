// Study Game Ability System _Hades

/**
 * @file ExecutionCalculation_Damage.cpp
 * @brief 伤害结算：技能基础伤害 + 施法者攻击力 → 目标防御减伤 → 写入 IncomingDamage。
 *
 * 数据流（谁提供什么）：
 *  - 技能（UAuraProjectileSpell::SpawnProjectile）在 Spec 上用 SetByCaller 写入两样东西：
 *    攻击类型 tag（Damage.Type.Physical / Damage.Type.Magical）作为 key，技能基础伤害作为 magnitude。
 *    同一个 Spec 可以同时写物理与魔法，构成混合伤害；
 *  - 本类负责把「基础伤害 + 施法者攻击力」算成原始伤害，再按受击者的护甲/魔抗减伤；
 *  - 结果只写入 IncomingDamage（Meta 属性），扣血、飘字、受击、死亡动画全部交给
 *    UAuraAttributeSet::PostGameplayEffectExecute，这里不直接改 Health；
 *  - 减伤曲线、防御刻度 K、等级反解都是纯函数，以 inline 形式放在
 *    ExecutionCalculation_Damage.h 的 AuraDamageMath namespace 里，本类直接调用。
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
#include "GameFramework/Actor.h"

// 独立日志分类：伤害结算每次命中都会走一遍，混进 LogTemp 会被引擎临时日志淹没。
// 默认 Log 级（Development / Shipping 都能看到）；想看"这次为什么没结算"再临时打开：
//     log LogAuraDamage Verbose
DEFINE_LOG_CATEGORY_STATIC(LogAuraDamage, Log, All);

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
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().MagicResistanceDef);
}

void UExecutionCalculation_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// 没有双方 ASC 的 GE（环境伤害、初始化属性）不具有「攻击 vs 防御」的语义，
	// 强行结算只会拿不到属性、刷一堆引擎警告，这里直接不产出伤害。
	if (!SourceASC || !TargetASC)
	{
		UE_LOG(LogAuraDamage, Verbose, TEXT("伤害结算跳过：SourceASC=%s TargetASC=%s"),
			SourceASC ? TEXT("有") : TEXT("无"), TargetASC ? TEXT("有") : TEXT("无"));
		return;
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// ========================================================================
	// 一、判定攻击类型 & 取技能基础伤害
	// ========================================================================
	// 约定：SetByCaller 的 key 就是攻击类型 tag，magnitude 是技能表里的基础伤害
	// （见 UAuraProjectileSpell::SpawnProjectile 里的 AssignTagSetByCallerMagnitude）。
	// 好处：「类型」和「数值」绑定在同一个 tag 上，不会出现"写了伤害忘了写类型"的错配。
	//
	// 第二个参数传 false：一个 Spec 本来就只带其中一种类型是常态，
	// 让引擎对"没找到"发警告会在每次物理命中时刷屏，属于误报。
	// 默认值传 0 而不是 -1：缺省即"这一段没有伤害"，后面统一用 > 0 判定。
	const float PhysicalBase = FMath::Max(Spec.GetSetByCallerMagnitude(Tags.Damage_Type_Physical, false, 0.f), 0.f);
	const float MagicalBase  = FMath::Max(Spec.GetSetByCallerMagnitude(Tags.Damage_Type_Magical,  false, 0.f), 0.f);

	const bool bHasPhysical = PhysicalBase > 0.f;
	const bool bHasMagical  = MagicalBase  > 0.f;

	// 两种都没配：不结算，避免"莫名其妙掉血"（也把漏写 SetByCaller 的 bug 暴露成 0 伤害而不是随机伤害）
	if (!bHasPhysical && !bHasMagical)
	{
		UE_LOG(LogAuraDamage, Verbose, TEXT("伤害结算跳过：Spec 上没有物理/魔法 SetByCaller 数值"));
		return;
	}

	// ========================================================================
	// 二、施法者攻击力加成
	// ========================================================================
	// 两段各自读对应的攻击属性：物理吃 PhysicalAttack，魔法吃 MagicalAttack。
	// AttemptCalculateCapturedAttributeMagnitude 失败时不会改写入参，
	// 所以先置 0，失败自然退化成"只按技能基础伤害结算"，而不是让整次伤害崩掉。
	float PhysicalAttack = 0.f;
	float MagicalAttack = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PhysicalAttackDef, EvaluateParameters, PhysicalAttack);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicalAttackDef, EvaluateParameters, MagicalAttack);
	PhysicalAttack = FMath::Max(PhysicalAttack, 0.f);
	MagicalAttack = FMath::Max(MagicalAttack, 0.f);

	// 加算而不是乘算：技能基础伤害横向比较技能强弱，攻击力纵向提供成长收益，两者解耦。
	const float RawPhysical = PhysicalBase + PhysicalAttack * AttackScaling;
	const float RawMagical  = MagicalBase + MagicalAttack * AttackScaling;

	// ========================================================================
	// 三、受击者防御减伤
	// ========================================================================
	float Armor = 0.f;
	float MagicResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParameters, Armor);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicResistanceDef, EvaluateParameters, MagicResistance);
	Armor = FMath::Max(Armor, 0.f);
	MagicResistance = FMath::Max(MagicResistance, 0.f);

	// K 的等级刻度跟施法者走（与减伤反向：K 越大减伤越弱）：
	//  - 跟受击者等级走时，受击者升级护甲只涨 1.5~4 点、K 却涨 10 点，减伤反而变低 → 升级更脆；
	//  - 跟施法者等级走时，同等级互打的 K 与旧实现逐字相同，受击者的等级强度只由护甲表达。
	// 施法者等级优先从 Avatar 反解（玩家在 PlayerState、敌人在 AuraCharacterBase::Level），
	// 拿不到时退回 Spec 等级——技能等级本身已经是角色等级（UAuraGameplayAbility::SetAbilityLevel）。
	const int32 AttackerLevel = AuraDamageMath::ResolveCharacterLevel(SourceASC->GetAvatarActor(), FMath::Max(FMath::RoundToInt(Spec.GetLevel()), 1));
	const float DefenseK = AuraDamageMath::ComputeDefenseK(DefenseBase, DefensePerLevel, AttackerLevel);

	// 受击者等级只用于日志核对（它通过护甲/魔抗影响结果，本身不再进 K）
	const int32 TargetLevel = AuraDamageMath::ResolveCharacterLevel(TargetASC->GetAvatarActor(), AttackerLevel);

	float FinalDamage = 0.f;
	if (bHasPhysical)
	{
		FinalDamage += AuraDamageMath::ApplyMitigation(RawPhysical, Armor, DefenseK, MinDamageRatio);
	}
	if (bHasMagical)
	{
		FinalDamage += AuraDamageMath::ApplyMitigation(RawMagical, MagicResistance, DefenseK, MinDamageRatio);
	}

	// ========================================================================
	// 四、输出 IncomingDamage（Meta 属性）
	// ========================================================================
	// 下游（扣血/飘字/受击/死亡）只处理 > 0 的伤害，因此这里再兜一次非负，
	// 负或 0 直接不产出 Modifier，省得下游走一遍"读到 0 就 return"。
	FinalDamage = FMath::Max(FinalDamage, 0.f);
	if (FinalDamage <= 0.f)
	{
		UE_LOG(LogTemp, Verbose, TEXT("伤害结算跳过：减伤后最终值为 0（原始 物理=%.2f 魔法=%.2f K=%.2f）"),
			RawPhysical, RawMagical, DefenseK);
		return;
	}

	// 这是唯一一条默认可见（Log 级）的日志：看到它就说明整条结算链路是通的。
	// %f 换成 %.2f 是为了让 Output Log 里的数字一眼能读，不用数小数点。
	UE_LOG(LogTemp, Log,
		TEXT("伤害结算 %s -> %s：物理基础=%.2f(攻击 %.2f / 护甲 %.2f) 魔法基础=%.2f(攻击 %.2f / 魔抗 %.2f) K=%.2f 等级=%d->%d 最终=%.2f"),
		*GetNameSafe(SourceASC->GetAvatarActor()), *GetNameSafe(TargetASC->GetAvatarActor()),
		PhysicalBase, PhysicalAttack, Armor,
		MagicalBase, MagicalAttack, MagicResistance,
		DefenseK, AttackerLevel, TargetLevel, FinalDamage);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}
