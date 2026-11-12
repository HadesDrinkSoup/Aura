// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "ExecutionCalculation_Damage.generated.h"

/**
 * 一次伤害结算需要的属性捕获定义集合：源方攻击力 + 目标方防御，共 4 个。
 *
 * 为什么捕获方式不一样：
 *  - 源属性 snapshot = true：伤害在「施法那一刻」定格，投射物还在飞行途中施法者吃了减益也不改这一发；
 *  - 目标属性 snapshot = false：命中那一瞬间取最新值，否则"飞行期间套上的护盾"不起作用。
 * 由 cpp 里的 static AuraDamageStatics 实例持有，避免每次结算都重建捕获列表。
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
	}
};

static const AuraDamageStatics& DamageStatics();

/**
 * 伤害结算 ExecutionCalculation：挂在「伤害 GE」的 Execution 上，每次施加对每个目标执行一次。
 * 公式（物理/魔法两段独立计算，可叠加成混合伤害）：
 *   物理段 RawDamage   = Damage.Physical.* 的基础伤害合计 + PhysicalAttack × AttackScaling
 *   魔法段 RawDamage   = Damage.Magical.*  的基础伤害合计 + MagicalAttack  × AttackScaling
 *   两段分别按各自防御减伤后相加：
 *   Defense     = 防御 / (防御 + 防御常量)
 *   FinalDamage = RawDamage × (1 - Defense)
 */
UCLASS()
class AURA_API UExecutionCalculation_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UExecutionCalculation_Damage();
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	float CalculateFinalDamage(const float Damage, const float Defense) const;
	
	// 伤害的转换系数
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Scaling", meta = (ClampMin = "0.0"))
	float AttackScaling = 0.6f;
	
	// 防御常量
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Defense", meta = (ClampMin = "1.0"))
	float DefenseConstant = 50.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Defense", meta = (ClampMin = "0.0"))
	// 暴击常量
	float CriticalAttackConstant = 1.5f;
};
