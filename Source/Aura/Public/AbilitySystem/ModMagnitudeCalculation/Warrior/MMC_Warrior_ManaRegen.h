// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_Warrior_ManaRegen.generated.h"

/**
 * 战士 · 法力恢复
 * 公式：0.5 + 智力 × 0.05 + 等级 × 0.02（钳制：最低 0）
 * 回蓝偏低：智力系数与等级系数都压得很低，满级也只有约 2.6 点/秒。
 *
 * 在系统里的角色：派生属性 ManaRegen 的 Magnitude 计算器。
 *  - 由 GE_DerivedAttributes_Warrior 中 ManaRegen 那条 Modifier 引用（Op = Add，Infinite）；
 *  - 语义是"每秒自动恢复的法力值"（见 AuraGamepalyTags.h 的 ManaRegen 注释），
 *    不包含技能/装备的主动回蓝；
 *  - 只捕获智力：回蓝是法系属性，战士不靠蓝量打循环，所以系数极小；
 *    与 MaxMana（智力 × 3.5）搭配成"池子小、回得慢"的组合，法术型技能对战士而言是稀缺资源；
 *  - 与 HealthRegen 的 0.18 活力系数对比：生命回复吃了主属性，法力回复只吃了次要属性，
 *    这就是"战士是近战、不是法术续航职业"的数值表达。
 *
 * 调参：数值全部来自下面 3 个 static constexpr，改完必须重新编译 C++ 才对 GE 生效。
 */
UCLASS()
class AURA_API UMMC_Warrior_ManaRegen : public UAuraMMCBase
{
	GENERATED_BODY()

public:
	/** 构造：登记本公式用到的属性捕获（仅智力，目标侧、非快照） */
	UMMC_Warrior_ManaRegen();

	/**
	 * 计算法力恢复 = 0.5 + 智力 × 0.05 + 等级 × 0.02，结果不低于 0。
	 * 由 GAS 在 GE 施加时及智力/等级变化后调用；纯函数。
	 * @param Spec 正在计算的 GE Spec，用于读取智力并解析角色等级
	 * @return 每秒法力恢复量（>= 0）
	 */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// 战士 · 法力恢复：0.5 + 智力 × 0.05 + 等级 × 0.02
	// 回蓝偏低：智力系数与等级系数都压得很低。
	/** 基础法力恢复（点/秒）：0.5 是"能慢慢回、但回不快"的底线，防止 L1 完全无回蓝 */
	static constexpr float BaseManaRegen = 0.5f;

	/** 智力系数：每 1 点智力 +0.05 点/秒（L50 智力 22 仅 +1.1），几乎不构成资源循环 */
	static constexpr float IntelligenceCoefficient = 0.05f;

	/** 等级系数：每级 +0.02 点/秒（L50 = +1），比生命回复的 0.06 低三倍，刻意压低法系续航 */
	static constexpr float LevelCoefficient = 0.02f;
};
