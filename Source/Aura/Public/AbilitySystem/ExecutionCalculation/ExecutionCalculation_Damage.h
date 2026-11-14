// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGamepalyTags.h"
#include "ExecutionCalculation_Damage.generated.h"

/**
 * 一次伤害结算需要的属性捕获定义集合：源方攻击力/命中/暴击 + 目标方防御/闪避 + 双方五行亲和。
 *
 * 为什么捕获方式不一样：
 *  - 源属性 snapshot = true：伤害在「施法那一刻」定格，投射物还在飞行途中施法者吃了减益也不改这一发；
 *  - 目标属性 snapshot = false：命中那一瞬间取最新值，否则"飞行期间套上的护盾"不起作用。
 * 由 cpp 里的函数内 static 实例持有，避免每次结算都重建捕获列表。
 */
struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalAttack)
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicalAttack)
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(HitChance)

	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor)
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DodgeChance)

	// 目标方亲和力：受击减伤用
	DECLARE_ATTRIBUTE_CAPTUREDEF(Wood)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Earth)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Lightning)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Water)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Fire)

	// 源方亲和力：出手增伤用。
	// 注意不能用 DEFINE_ATTRIBUTE_CAPTUREDEF 定义：那个宏按「属性名 + Def」命名，
	// 同一个亲和属性已经给目标方定义过一次，再定义会覆盖掉 Target 那一份。
	// 所以这里显式声明，构造函数里用 FGameplayEffectAttributeCaptureDefinition 手工构造。
	FGameplayEffectAttributeCaptureDefinition SourceWoodDef;
	FGameplayEffectAttributeCaptureDefinition SourceEarthDef;
	FGameplayEffectAttributeCaptureDefinition SourceLightningDef;
	FGameplayEffectAttributeCaptureDefinition SourceWaterDef;
	FGameplayEffectAttributeCaptureDefinition SourceFireDef;

	/** 抗性标签（Attributes.Affinity.*）→ 目标方捕获定义：受击减伤用 */
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToAttributesCaptureDefs;

	/** 伤害元素 tag（Damage.Element.*）→ 源方捕获定义：出手增伤用 */
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> ElementToSourceAffinityDefs;

	/**
	 * 用「已经注册好的」native GameplayTag 重建两张标签映射表。
	 *
	 * 为什么不能只靠构造函数：本结构体是函数内 static，第一次构造发生在
	 * UExecutionCalculation_Damage 的 CDO 构造里（模块加载期），早于
	 * UAuraAssetManager::StartInitialLoading → FAuraGameplayTags::InitializeNativeGameplayTags。
	 * 构造期 Add 进去的 key 全是无效 tag，且同名 key 会相互覆盖，两张表最终只剩一条空 key：
	 *   TagsToAttributesCaptureDefs.Find(Attributes.Affinity.Water)  → nullptr
	 *   ElementToSourceAffinityDefs.Find(Damage.Element.Fire)        → nullptr
	 * 于是 Execute 里读不到任何亲和值，增伤/减伤/弱点恒为 0，元素系数恒为 1 —— 元素系统静默失效。
	 *
	 * 所以构造函数末尾调用一次（tag 已注册就当场建好），首次 Execute 再调用一次兜底；
	 * 建好后 bTagMapsBuilt 直接短路。若 tag 尚未注册，保持未建状态，下次结算重试。
	 */
	void EnsureTagMapsBuilt();

private:
	/** 标签表是否已用真实标签建好（false = 表里只有空 key 或尚未建） */
	bool bTagMapsBuilt = false;

public:
	AuraDamageStatics()
	{
		// 源属性：snapshot = true，伤害在施法那一刻定格，飞行途中 buff 过期不改数
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalAttack, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, MagicalAttack, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, HitChance, Source, true);
		// 目标属性：snapshot = false，命中那一瞬间取最新值
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, MagicResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, DodgeChance, Target, false);

		// 目标方五行亲和：同样是实时取值
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Wood, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Earth, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Lightning, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Water, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Fire, Target, false);

		// 源方五行亲和：和攻击力一样 snapshot = true，施法那一刻定格
		SourceWoodDef = FGameplayEffectAttributeCaptureDefinition(UAuraAttributeSet::GetWoodAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
		SourceEarthDef = FGameplayEffectAttributeCaptureDefinition(UAuraAttributeSet::GetEarthAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
		SourceLightningDef = FGameplayEffectAttributeCaptureDefinition(UAuraAttributeSet::GetLightningAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
		SourceWaterDef = FGameplayEffectAttributeCaptureDefinition(UAuraAttributeSet::GetWaterAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
		SourceFireDef = FGameplayEffectAttributeCaptureDefinition(UAuraAttributeSet::GetFireAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static AuraDamageStatics& DamageStatics();

/**
 * 伤害结算 ExecutionCalculation：挂在「伤害 GE」的 Execution 上，每次施加对每个目标执行一次。
 * 公式（先按防御分段，再统一做一次元素修正，最后暴击）：
 *
 *   1) 伤害通道（决定「怎么算」）
 *   物理段 RawDamage = Bonus.Physical 的基础伤害 + PhysicalAttack × AttackScaling
 *   魔法段 RawDamage = Bonus.Magical  的基础伤害 + MagicalAttack  × AttackScaling
 *   两段分别按各自防御减伤：
 *   Defense       = 防御 / (防御 + DefenseConstant)
 *   SegmentDamage = RawDamage × (1 - Defense)
 *   段合计 = 物理段 + 魔法段
 *
 *   2) 本次攻击的元素 E 唯一来源是效果上下文的 DamageType（Damage.Element.*），
 *      由技能（UAuraProjectileSpell）在施法时写进上下文。技能标了元素，这一击打出去的
 *      全部伤害就是该元素；没标就不是元素攻击，不做元素修正（倍率 = 1）。
 *      元素不需要、也不应该再配一条 Damage.Element.* 的 SetByCaller 伤害。
 *
 *   3) 元素修正（一次结算只有一个元素，受击者的五项亲和都参与）
 *   先统一基准：有效亲和 = max(亲和 - AffinityBaseline, 0)
 *   通用灵属 GE 给所有角色的五项亲和都 +10，这 10 点是「没有偏好」的基线，不是免费伤害/免伤；
 *   只有把某一项堆到基线以上（或让对手的弱点项高于基线）才会真正改变倍率。
 *   出手增伤 +Gain = 有效亲和(施法者, E) / (有效亲和 + ElementalMasteryConstant) × ElementalMasteryStrength
 *   受击减伤 -Mitigation = 有效亲和(受击者, 克制E) / (… + ElementalResistanceConstant) × ElementalResistanceStrength
 *   受击增伤 +Weakness  = 有效亲和(受击者, 被E克制，即它生E) / (… + ElementalWeaknessConstant) × ElementalWeaknessStrength
 *   克制关系都从 DamageTypesToResistances 读（火伤害 → 水亲和减伤；火克木 → 木亲和增伤）
 *   ElementMultiplier = clamp(1 + Gain - Mitigation + Weakness, ElementalMultiplierMin, ElementalMultiplierMax)
 *   FinalDamage = 段合计 × ElementMultiplier × 暴击倍率
 *
 * 说明：通道（物理/魔法）管公式，元素管增减伤，两者正交；
 * 一个技能只标一个元素，所以整次结算只做一次元素修正。
 */
UCLASS()
class AURA_API UExecutionCalculation_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UExecutionCalculation_Damage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	float CalculateFinalDamage(const float Damage, const float Defense) const;

	/** 元素出手增伤比例（0~1）：施法者该元素「高于基准」的亲和越高，打出的该元素伤害越高 */
	float CalculateElementMasteryRatio(float Affinity) const;

	/** 元素受击减伤比例（0~1）：受击者「克制该元素」的亲和高于基准越多，受到的该元素伤害越低 */
	float CalculateElementResistancesRatio(float Affinity) const;

	/** 元素受击增伤比例（0~1）：受击者「被该元素克制」的亲和高于基准越多，受到的该元素伤害越高（弱点） */
	float CalculateElementWeaknessRatio(float Affinity) const;

	// 伤害的转换系数
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Scaling", meta = (DisplayName = "攻击力系数", ClampMin = "0.0"))
	float AttackScaling = 0.6f;

	// 防御常量
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Defense", meta = (DisplayName = "防御常数", ClampMin = "1.0"))
	float DefenseConstant = 50.f;

	/**
	 * 元素亲和基准值。
	 * 通用灵属 GE（GE_ElementAffinityBase）给所有角色的五项亲和都 +10，
	 * 这 10 点代表「凡人对五行都有基础感知」，不应该直接换成伤害/免伤，
	 * 所以结算前先减掉基准：有效亲和 = max(亲和 - AffinityBaseline, 0)。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Element", meta = (DisplayName = "元素亲和基准", ClampMin = "0.0"))
	float AffinityBaseline = 5.f;

	/**
	 * 元素抗性常量（受击侧）。
	 * 有效亲和 10（即面板 20）时减伤 33.3%，有效 20（面板 30）时 50%，有效 40（面板 50）时 66.7%；
	 * 亲和力越高减伤越高但永不免疫，避免「堆满某项亲和力就完全免疫某个属性」的线性百分比模型。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Defense", meta = (DisplayName = "元素抗性常量", ClampMin = "1.0"))
	float ElementalResistanceConstant = 20.f;

	/**
	 * 元素精通常量（出手侧）。
	 * 与 ElementalResistanceConstant 分开配置：想「亲和力主要用来防御」就把这个调大，
	 * 想「亲和力同时明显提升输出」就调小。默认两边都是 20，所以双方堆到同一档时增伤与减伤抵消。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Offense", meta = (DisplayName = "元素精通常量", ClampMin = "1.0"))
	float ElementalMasteryConstant = 20.f;

	/** 出手增伤的强度系数（1 = 按比例全额生效，0 = 关闭出手侧增伤，只保留受击减伤） */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Offense", meta = (DisplayName = "元素精通强度", ClampMin = "0.0", ClampMax = "5.0"))
	float ElementalMasteryStrength = 1.f;

	/** 受击减伤的强度系数（1 = 全额，0 = 关闭元素防御，元素只影响输出） */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Defense", meta = (DisplayName = "元素抗性强度", ClampMin = "0.0", ClampMax = "5.0"))
	float ElementalResistanceStrength = 1.f;

	/**
	 * 元素弱点常量（受击侧，增伤）。
	 * 受击者「被本元素克制」的那项亲和越高，受到的该元素伤害越高（火打木 → 木亲和高的人更疼）。
	 * 常数比抗性大（25 vs 20）、强度再乘 0.75，让弱点没有抗性那么极端：
	 * 木 30 挨火是 +33.3%，水 30 挨火是 -50%，攻守不对称但都在可读范围内。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Defense", meta = (DisplayName = "元素弱点常量", ClampMin = "1.0"))
	float ElementalWeaknessConstant = 25.f;

	/** 受击增伤（弱点）的强度系数：1 = 全额，0 = 关闭弱点，只保留克制减伤；默认 0.75 让弱点略钝 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Defense", meta = (DisplayName = "元素弱点强度", ClampMin = "0.0", ClampMax = "5.0"))
	float ElementalWeaknessStrength = 0.75f;

	/** 元素系数下限：极端被克制时的最低倍率（默认 0.25 = 只吃 25% 伤害），防止元素伤害被压成 0 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Element", meta = (DisplayName = "元素系数下限", ClampMin = "0.0", ClampMax = "1.0"))
	float ElementalMultiplierMin = 0.25f;

	/** 元素系数上限：极端克制时的最高倍率（默认 2.0 = 双倍伤害），防止一发秒杀 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Element", meta = (DisplayName = "元素系数上限", ClampMin = "1.0", ClampMax = "5.0"))
	float ElementalMultiplierMax = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Aura|Damage|Defense", meta = (DisplayName = "暴击伤害系数", ClampMin = "0.0"))
	// 暴击常量
	float CriticalAttackConstant = 1.5f;
};
