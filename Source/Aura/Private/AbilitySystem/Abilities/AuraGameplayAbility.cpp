// Study Game Ability System _Hades

/**
 * @file AuraGameplayAbility.cpp
 * @brief 技能基类 UAuraGameplayAbility 的翻译单元。
 *
 * 要点：
 *  - 本文件只实现一个查询函数 SetAbilityLevel：把「技能等级」解析成「施法者角色等级」，
 *    不重写 ActivateAbility / EndAbility / CanActivateAbility 等虚函数，激活时序仍由子类与蓝图负责。
 *  - 能力逻辑的实际分布：
 *      · C++ 子类 UAuraProjectileSpell（见同目录 AuraProjectileSpell.cpp）；
 *      · 蓝图子类 GA_FireBall 等（Content/Blueprint/AbilitySystem/Abilities/，由 UHT 反射出的
 *        蓝图节点调用 C++ 侧接口，例如 UAuraProjectileSpell::SpawnProjectile）。
 *  - 输入 Tag 的含义、由谁写入 AbilitySpec、由谁派发，均见头文件里 AbilityInputTag 的说明。
 */

#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"

int32 UAuraGameplayAbility::SetAbilityLevel() const
{
	// 角色等级与 MMC、初始 GE 走同一来源（玩家在 AuraPlayerState::Level，敌人在 AuraCharacterBase::Level），
	// 这样「技能等级」和「属性成长用的等级」永远指同一件事，不会出现两套等级各长各的。
	if (const AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		if (const ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarActor))
		{
			if (const int32 CharacterLevel = CombatInterface->GetCharacterLevel(); CharacterLevel > 0)
			{
				return CharacterLevel;
			}
		}
	}

	// 兜底：施法者没有等级概念（或技能还在预授予阶段拿不到 Avatar）时退回 Spec 等级，
	// GetAbilityLevel() 在 UE 里返回 FGameplayAbilitySpec::Level，授予时写入，至少是 1。
	return FMath::Max(GetAbilityLevel(), 1);
}
