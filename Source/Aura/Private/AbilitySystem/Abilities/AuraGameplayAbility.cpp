// Study Game Ability System _Hades

/**
 * @file AuraGameplayAbility.cpp
 * @brief 技能基类 UAuraGameplayAbility 的翻译单元。
 *
 * 要点：
 *  - 本文件只有 include，没有任何函数实现：基类相对 UGameplayAbility 只新增了一个 UPROPERTY
 *    （AbilityInputTag），既没有自定义构造函数，也没有重写 ActivateAbility / EndAbility /
 *    CanActivateAbility 等虚函数，因此不存在需要在这里实现的函数体。
 *  - 能力逻辑的实际分布：
 *      · C++ 子类 UAuraProjectileSpell（见同目录 AuraProjectileSpell.cpp）；
 *      · 蓝图子类 GA_FireBall 等（Content/Blueprint/AbilitySystem/Abilities/，由 UHT 反射出的
 *        蓝图节点调用 C++ 侧接口，例如 UAuraProjectileSpell::SpawnProjectile）。
 *  - 输入 Tag 的含义、由谁写入 AbilitySpec、由谁派发，均见头文件里 AbilityInputTag 的说明。
 */

#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
