// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * Aura 所有技能类的基类（C++ 侧目前只有 UAuraProjectileSpell，蓝图侧是 GA_FireBall 等）。
 *
 * 相对 UGameplayAbility 只多了一个数据成员 AbilityInputTag——GAS 本身不认识"输入"，
 * 它只认识能力实例（FGameplayAbilitySpec）；项目用这个 Tag 把"哪个输入动作 → 哪个技能"
 * 的映射存进 Spec，再在运行时按 Tag 派发输入事件：
 *   - 授予技能时：UAuraAbilitySystemComponent::AddCharacterAbilities 把 AbilityInputTag
 *     通过 GetDynamicSpecSourceTags().AddTag() 挂到 Spec 的动态来源标签上；
 *   - 派发输入时：AbilityInputTagPressed / AbilityInputTagReleased / AbilityInputTagHeld
 *     遍历已授予能力，用 HasTagExact 命中后调用 AbilitySpecInputPressed / AbilitySpecInputReleased，
 *     这三个函数由 AAuraPlayerController 的输入回调（Enhanced Input）转发进来。
 *
 * 设计要点：
 *  - 本类不重写任何 UGameplayAbility 的虚函数，也不实现构造逻辑，能力逻辑一律写在子类里；
 *  - 它同时是 UAuraAbilitySystemComponent::AddCharacterAbilities 里 CastChecked<UAuraGameplayAbility>
 *    的类型约束——没有继承本类的 UGameplayAbility 一旦放进技能列表，授予时会直接断言失败。
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	/**
	 * 技能对应的输入标签：只用于输入分发，不参与技能能否激活的判定
	 * （阻塞、成本、冷却由 GE / GameplayTag 另管，见 UGameplayAbility 的 ActivationBlockedTags 等）。
	 *
	 * 取值必须与 UInputConfig 里配的输入 Tag 完全相同：输入回调把同一个 Tag 传给 ASC，
	 * 匹配用的是 HasTagExact（精确相等），因此父标签、子标签都不会命中。
	 * EditDefaultsOnly：只能在类默认值层面配置（C++ 构造 / 蓝图类默认值），不能在关卡里的单实例上改。
	 * 留空则该技能永远不会被输入触发，只能由代码或蓝图直接 TryActivateAbility。
	 */
	UPROPERTY(EditDefaultsOnly, Category="AbilityInput")
	FGameplayTag AbilityInputTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AbilityDamage")
	FScalableFloat Damage;
};