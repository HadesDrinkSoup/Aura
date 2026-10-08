// Study Game Ability System _Hades

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Aura GameplayTags 集中管理结构体
 * 用于统一注册和访问项目所有 GameplayTag，避免硬编码字符串
 */
struct FAuraGameplayTags
{
	static const FAuraGameplayTags& Get() {return GameplayTags;}
	static void InitializeNativeGameplayTags();

	/** Pool Attributes — 运行时动态变动的当前值，直接受 GameplayEffect 修改 */
	FGameplayTag Attributes_Pool_Health;   // 当前生命值
	FGameplayTag Attributes_Pool_Mana;     // 当前法力值

	/** Primary Attributes — 基础属性，由装备/等级等系统直接修改，被 MMC 捕获作为派生属性计算的输入 */
	FGameplayTag Attributes_Primary_Strength;     // 力量 — 影响物理攻击力、最大生命值、护甲
	FGameplayTag Attributes_Primary_Intelligence; // 智力 — 影响魔法攻击力、最大法力值、魔法抗性、暴击率、冷却缩减、法力恢复
	FGameplayTag Attributes_Primary_Agility;      // 敏捷 — 影响物理攻击力、暴击率、命中率、闪避率、攻击速度、移动速度
	FGameplayTag Attributes_Primary_Vigor;        // 活力 — 影响最大生命值、最大法力值、护甲、魔法抗性、生命恢复

	/** Derived Attributes — 派生属性，由 MMC 根据 Primary 属性计算得出，通过 GameplayEffect 写入 */
	FGameplayTag Attributes_Derived_MaxHealth;         // 最大生命值 — 受 Vigor + Strength + 等级 影响，决定角色生命上限
	FGameplayTag Attributes_Derived_MaxMana;           // 最大法力值 — 受 Vigor + Intelligence + 等级 影响，决定角色法力上限
	FGameplayTag Attributes_Derived_PhysicalAttack;    // 物理攻击力 — 受 Strength + Agility + 等级 影响，决定物理伤害基础数值
	FGameplayTag Attributes_Derived_MagicalAttack;     // 魔法攻击力 — 受 Intelligence + 等级 影响，决定魔法伤害基础数值
	FGameplayTag Attributes_Derived_Armor;             // 护甲 — 受 Vigor + Strength + 等级 影响，降低受到的物理伤害
	FGameplayTag Attributes_Derived_MagicResistance;   // 魔法抗性 — 受 Vigor + Intelligence + 等级 影响，降低受到的魔法伤害
	FGameplayTag Attributes_Derived_CriticalChance;    // 暴击几率 — 受 Agility + Intelligence 影响，决定攻击暴击的概率
	FGameplayTag Attributes_Derived_HitChance;         // 命中几率 — 受 Agility + 等级 影响，决定攻击命中目标的概率
	FGameplayTag Attributes_Derived_DodgeChance;       // 闪避几率 — 受 Agility 影响，决定角色闪避攻击的概率
	FGameplayTag Attributes_Derived_AttackSpeed;       // 攻击速度 — 受 Agility + 等级 影响，决定角色攻击频率
	FGameplayTag Attributes_Derived_CooldownReduction; // 冷却缩减 — 受 Intelligence 影响，降低技能冷却时间
	FGameplayTag Attributes_Derived_HealthRegen;       // 生命恢复 — 受 Vigor + 等级 影响，决定每秒自动恢复的生命值
	FGameplayTag Attributes_Derived_ManaRegen;         // 法力恢复 — 受 Intelligence + 等级 影响，决定每秒自动恢复的法力值
	FGameplayTag Attributes_Derived_MoveSpeed;         // 移动速度 — 受 Agility 影响，决定角色移动速度

private:
	static FAuraGameplayTags GameplayTags;
};
