// Study Game Ability System _Hadess

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraDamageGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
	
protected:
	//命中时施加的伤害GE类
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "Aura|Ability|Damage", meta = (DisplayName = "伤害效果类"))
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	//这一击的类型（如 Damage.Element.Fire）：随效果上下文传给结算之后的所有环节，不参与伤害数值
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Ability|Damage", meta = (DisplayName = "伤害类型"))
	FGameplayTag DamageType;

	//这一击的数值：key 是 SetByCaller 用的标签（Bonus.Physical / Bonus.Magical / Damage.Element.*），
	//value 是按技能等级取值的曲线。结算端按 key 分流到对应抗性
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Ability|Damage", meta = (DisplayName = "伤害加成类型"))
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
};
