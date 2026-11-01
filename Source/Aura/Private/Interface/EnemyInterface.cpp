// Study Game Ability System _Hades
//
// @file  EnemyInterface.cpp
// @brief 敌人接口的编译单元；接口本身没有可执行实现。
//
// 本文件不定义任何逻辑：HighlightActor / UnHighlightActor 在头文件中给出空实现，
// 由 AAuraEnemy 重写为"开关 Custom Depth + 写模板值"的具体行为。
// 保留该 .cpp 是为了让头文件被独立编译，及早发现 include 缺失。


#include "Interface/EnemyInterface.h"

// Add default functionality here for any IEnemyInterface functions that are not pure virtual.
// 上面这行是 UBT 模板生成的提示：本接口的虚函数在头文件里已给出默认实现（空函数体），
// 若以后新增非纯虚函数，可以在这个位置补默认行为。
