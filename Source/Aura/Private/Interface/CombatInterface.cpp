// Study Game Ability System _Hades
//
// @file  CombatInterface.cpp
// @brief 战斗接口的编译单元；接口本身没有可执行实现。
//
// 本文件不定义任何逻辑：
//  - ICombatInterface 的成员都是带默认实现的虚函数（内联在头文件里），
//    具体行为由 AAuraCharacterBase / AAuraCharacter / AAuraEnemy 重写；
//  - UpdateFacingTarget 是 BlueprintImplementableEvent，实现内容在蓝图里。
// 保留该 .cpp 的意义是让头文件被单独编译一次，从而尽早暴露缺失的 include。


#include "Interface/CombatInterface.h"
