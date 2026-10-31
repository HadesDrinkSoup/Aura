#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""按统一数值表生成三职业（Elementalist / Warrior / Ranger）的 MMC 类与设计文档。

用法：
    python Tools/ModMagnitudeCalculation/generate_mmc.py

说明：
    - 本脚本只用于「初始化 / 批量重排」，生成后的 .h / .cpp 可以直接手改；
      再次运行会覆盖同名文件，因此手改后请不要重复执行。
    - 数值表的唯一来源就是下面的 STATS / CLASSES，改数值时两边保持一致即可。
"""

from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
MODULE_ROOT = PROJECT_ROOT / "Source" / "Aura"
PUBLIC_MMC = MODULE_ROOT / "Public" / "AbilitySystem" / "ModMagnitudeCalculation"
PRIVATE_MMC = MODULE_ROOT / "Private" / "AbilitySystem" / "ModMagnitudeCalculation"
DOC_PATH = PROJECT_ROOT / "Docs" / "ModMagnitudeCalculation.md"

# ============================================================================
# 基础属性定义：C++ 变量名 / 获取函数 / 中文名 / 系数常量名
# ============================================================================
ATTRS = {
    "Strength": {
        "var": "Strength",
        "getter": "GetStrengthAttribute",
        "coef": "StrengthCoefficient",
        "cn": "力量",
    },
    "Intelligence": {
        "var": "Intelligence",
        "getter": "GetIntelligenceAttribute",
        "coef": "IntelligenceCoefficient",
        "cn": "智力",
    },
    "Agility": {
        "var": "Agility",
        "getter": "GetAgilityAttribute",
        "coef": "AgilityCoefficient",
        "cn": "敏捷",
    },
    "Vigor": {
        "var": "Vigor",
        "getter": "GetVigorAttribute",
        "coef": "VigorCoefficient",
        "cn": "活力",
    },
}

# ============================================================================
# 职业定义
# ============================================================================
CLASSES = [
    {
        "key": "Elementalist",
        "cn": "元素法师",
        "primary": "智力",
        "identity": "智力主导的法术输出：法力池、法强、冷却缩减、回蓝、魔抗最高；生命、护甲、攻速最低。",
        "strengths": "法强 / 法力池 / 冷却缩减 / 回蓝 / 魔抗",
        "weaknesses": "生命 / 护甲 / 攻速 / 闪避",
    },
    {
        "key": "Warrior",
        "cn": "战士",
        "primary": "力量 + 活力",
        "identity": "力量与活力主导的前排：生命、护甲、物攻、回血最高；法力与冷却缩减最低。",
        "strengths": "生命 / 护甲 / 物攻 / 回血",
        "weaknesses": "法力 / 法强 / 冷却缩减 / 移速",
    },
    {
        "key": "Ranger",
        "cn": "游侠",
        "primary": "敏捷",
        "identity": "敏捷主导的远程输出：攻速、暴击、闪避、命中、移速最高；护甲与生命偏低。",
        "strengths": "攻速 / 暴击 / 闪避 / 命中 / 移速",
        "weaknesses": "护甲 / 生命 / 法强",
    },
]

# ============================================================================
# 属性计算表
#   base  : 基础值
#   terms : [(基础属性, 系数), ...]，按书写顺序输出
#   level : 等级系数（0 表示不参与计算）
#   lo/hi : 结果钳制范围（hi=None 表示只保底）
# ============================================================================
STATS = [
    {
        "key": "MaxHealth",
        "cn": "最大生命值",
        "base_const": "BaseHealth",
        "getter": "GetMaxHealthAttribute",
        "classes": {
            "Elementalist": dict(base=65, terms=[("Vigor", 7), ("Strength", 2)], level=7, lo=1, hi=None,
                                 note="体魄最弱：血量主要来自等级，活力收益明显低于战士。"),
            "Warrior": dict(base=140, terms=[("Vigor", 15), ("Strength", 9)], level=15, lo=1, hi=None,
                            note="血量最高：活力与力量双收益，是前排承伤的核心。"),
            "Ranger": dict(base=90, terms=[("Vigor", 9), ("Strength", 4)], level=9, lo=1, hi=None,
                           note="血量中等：靠闪避与走位生存，而不是硬抗伤害。"),
        },
    },
    {
        "key": "MaxMana",
        "cn": "最大法力值",
        "base_const": "BaseMana",
        "getter": "GetMaxManaAttribute",
        "classes": {
            "Elementalist": dict(base=120, terms=[("Intelligence", 15), ("Vigor", 2)], level=9, lo=0, hi=None,
                                 note="法力池最大，智力收益最高，支撑长时间施法。"),
            "Warrior": dict(base=25, terms=[("Intelligence", 3.5), ("Vigor", 1)], level=2.5, lo=0, hi=None,
                            note="法力池最小，只够支撑少量战技。"),
            "Ranger": dict(base=55, terms=[("Intelligence", 6.5), ("Vigor", 1.5)], level=4.5, lo=0, hi=None,
                           note="法力中等，足以支撑标记与位移类技能。"),
        },
    },
    {
        "key": "PhysicalAttack",
        "cn": "物理攻击力",
        "base_const": "BasePhysicalAttack",
        "getter": "GetPhysicalAttackAttribute",
        "classes": {
            "Elementalist": dict(base=3, terms=[("Strength", 1), ("Agility", 0.3)], level=0.6, lo=0, hi=None,
                                 note="几乎不依赖武器，物理攻击只作兜底输出。"),
            "Warrior": dict(base=12, terms=[("Strength", 3.2), ("Agility", 0.4)], level=1.5, lo=0, hi=None,
                            note="核心输出：力量收益三职业最高。"),
            "Ranger": dict(base=8, terms=[("Strength", 1.3), ("Agility", 2.4)], level=1.2, lo=0, hi=None,
                           note="敏捷是主要加成，力量提供基础底子。"),
        },
    },
    {
        "key": "MagicalAttack",
        "cn": "魔法攻击力",
        "base_const": "BaseMagicalAttack",
        "getter": "GetMagicalAttackAttribute",
        "classes": {
            "Elementalist": dict(base=12, terms=[("Intelligence", 3.4)], level=1.5, lo=0, hi=None,
                                 note="核心输出：智力收益三职业最高。"),
            "Warrior": dict(base=2, terms=[("Intelligence", 0.8)], level=0.3, lo=0, hi=None,
                            note="几乎无魔法输出，智力收益极低。"),
            "Ranger": dict(base=4, terms=[("Intelligence", 1.5), ("Agility", 0.4)], level=0.7, lo=0, hi=None,
                           note="魔法输出为辅，敏捷提供少量额外法强。"),
        },
    },
    {
        "key": "Armor",
        "cn": "护甲",
        "base_const": "BaseArmor",
        "getter": "GetArmorAttribute",
        "classes": {
            "Elementalist": dict(base=0, terms=[("Vigor", 0.8), ("Strength", 0.2)], level=0.15, lo=0, hi=None,
                                 note="基础护甲为 0，只能获得极低的物理减免。"),
            "Warrior": dict(base=5, terms=[("Vigor", 1.8), ("Strength", 1.3)], level=0.55, lo=0, hi=None,
                            note="护甲最高：活力 + 力量双加成，物理减伤核心。"),
            "Ranger": dict(base=1, terms=[("Vigor", 1), ("Strength", 0.35)], level=0.25, lo=0, hi=None,
                           note="护甲偏低，被近身时风险很高。"),
        },
    },
    {
        "key": "MagicResistance",
        "cn": "魔法抗性",
        "base_const": "BaseMagicResistance",
        "getter": "GetMagicResistanceAttribute",
        "classes": {
            "Elementalist": dict(base=3, terms=[("Vigor", 0.6), ("Intelligence", 1.2)], level=0.4, lo=0, hi=None,
                                 note="智力兼顾魔抗，法系内战中抗性最好。"),
            "Warrior": dict(base=1, terms=[("Vigor", 0.8), ("Intelligence", 0.15)], level=0.15, lo=0, hi=None,
                            note="魔抗偏低，需要装备弥补法术承伤。"),
            "Ranger": dict(base=2, terms=[("Vigor", 0.5), ("Intelligence", 0.5)], level=0.25, lo=0, hi=None,
                           note="魔抗中等，活力与智力均衡提供。"),
        },
    },
    {
        "key": "CriticalChance",
        "cn": "暴击几率",
        "base_const": "BaseCriticalChance",
        "getter": "GetCriticalChanceAttribute",
        "classes": {
            "Elementalist": dict(base=5, terms=[("Intelligence", 0.1), ("Agility", 0.05)], level=0, lo=0, hi=75,
                                 note="暴击主要来自智力，敏捷只提供少量补充。"),
            "Warrior": dict(base=5, terms=[("Agility", 0.08), ("Strength", 0.04)], level=0, lo=0, hi=75,
                            note="暴击来自敏捷与力量的稳定加成，成长平缓。"),
            "Ranger": dict(base=8, terms=[("Agility", 0.2), ("Intelligence", 0.03)], level=0, lo=0, hi=75,
                           note="暴击核心职业：敏捷收益三职业最高。"),
        },
    },
    {
        "key": "HitChance",
        "cn": "命中几率",
        "base_const": "BaseHitChance",
        "getter": "GetHitChanceAttribute",
        "classes": {
            "Elementalist": dict(base=80, terms=[("Agility", 0.06), ("Intelligence", 0.05)], level=0.1, lo=0, hi=100,
                                 note="命中偏低，需要装备或天赋补足。"),
            "Warrior": dict(base=82, terms=[("Agility", 0.08)], level=0.15, lo=0, hi=100,
                            note="命中稳定偏高，保证近战技能不落空。"),
            "Ranger": dict(base=88, terms=[("Agility", 0.18)], level=0.2, lo=0, hi=100,
                           note="命中最高，远程攻击的稳定性是核心优势。"),
        },
    },
    {
        "key": "DodgeChance",
        "cn": "闪避几率",
        "base_const": "BaseDodgeChance",
        "getter": "GetDodgeChanceAttribute",
        "classes": {
            "Elementalist": dict(base=2, terms=[("Agility", 0.08)], level=0, lo=0, hi=60,
                                 note="闪避成长最差，不靠敏捷生存。"),
            "Warrior": dict(base=3, terms=[("Agility", 0.07)], level=0, lo=0, hi=60,
                            note="闪避成长最低，靠护甲而不是闪避生存。"),
            "Ranger": dict(base=6, terms=[("Agility", 0.18)], level=0, lo=0, hi=60,
                           note="闪避核心职业：敏捷收益最高。"),
        },
    },
    {
        "key": "AttackSpeed",
        "cn": "攻击速度",
        "base_const": "BaseAttackSpeed",
        "getter": "GetAttackSpeedAttribute",
        "classes": {
            "Elementalist": dict(base=95, terms=[("Agility", 0.15)], level=0.1, lo=50, hi=250,
                                 note="基础攻速最低，普攻不是主要输出手段。"),
            "Warrior": dict(base=100, terms=[("Agility", 0.2)], level=0.1, lo=50, hi=250,
                            note="攻速中等，重视单次伤害而非攻击频率。"),
            "Ranger": dict(base=110, terms=[("Agility", 0.45)], level=0.2, lo=50, hi=250,
                           note="攻速最高，高频普攻与技能衔接是核心玩法。"),
        },
    },
    {
        "key": "CooldownReduction",
        "cn": "冷却缩减",
        "base_const": "BaseCooldownReduction",
        "getter": "GetCooldownReductionAttribute",
        "classes": {
            "Elementalist": dict(base=3, terms=[("Intelligence", 0.25)], level=0.1, lo=0, hi=40,
                                 note="冷却缩减核心职业，智力收益最高。"),
            "Warrior": dict(base=0, terms=[("Intelligence", 0.08)], level=0.03, lo=0, hi=40,
                            note="冷却缩减最低，技能循环明显偏慢。"),
            "Ranger": dict(base=1, terms=[("Agility", 0.1), ("Intelligence", 0.05)], level=0, lo=0, hi=40,
                           note="冷却缩减一般，主要来自敏捷。"),
        },
    },
    {
        "key": "HealthRegen",
        "cn": "生命恢复",
        "base_const": "BaseHealthRegen",
        "getter": "GetHealthRegenAttribute",
        "classes": {
            "Elementalist": dict(base=0.5, terms=[("Vigor", 0.08)], level=0.03, lo=0, hi=None,
                                 note="基础回血最低，续航依赖法力与技能。"),
            "Warrior": dict(base=1.5, terms=[("Vigor", 0.18)], level=0.06, lo=0, hi=None,
                            note="回血最高，配合高血量形成持续作战能力。"),
            "Ranger": dict(base=0.8, terms=[("Vigor", 0.1)], level=0.04, lo=0, hi=None,
                           note="回血中等，续航依赖脱战与闪避。"),
        },
    },
    {
        "key": "ManaRegen",
        "cn": "法力恢复",
        "base_const": "BaseManaRegen",
        "getter": "GetManaRegenAttribute",
        "classes": {
            "Elementalist": dict(base=3, terms=[("Intelligence", 0.3)], level=0.1, lo=0, hi=None,
                                 note="回蓝核心职业，支撑高频技能循环。"),
            "Warrior": dict(base=0.5, terms=[("Intelligence", 0.05)], level=0.02, lo=0, hi=None,
                            note="回蓝最低，法力属于稀缺资源。"),
            "Ranger": dict(base=1, terms=[("Intelligence", 0.12), ("Agility", 0.04)], level=0.04, lo=0, hi=None,
                           note="回蓝中等，敏捷提供少量额外回复。"),
        },
    },
    {
        "key": "MoveSpeed",
        "cn": "移动速度",
        "base_const": "BaseMoveSpeed",
        "getter": "GetMoveSpeedAttribute",
        "classes": {
            "Elementalist": dict(base=380, terms=[("Agility", 0.25)], level=0, lo=0, hi=None,
                                 note="移速最慢，靠位移技能弥补机动性。"),
            "Warrior": dict(base=390, terms=[("Agility", 0.3), ("Strength", 0.05)], level=0, lo=0, hi=None,
                            note="移速中等，力量提供极少额外移速。"),
            "Ranger": dict(base=420, terms=[("Agility", 0.55)], level=0, lo=0, hi=None,
                           note="移速最快，风筝与拉扯是核心优势。"),
        },
    },
]


# ============================================================================
# 渲染辅助
# ============================================================================
def cpp_float(value) -> str:
    """把 Python 数值渲染成 UE 风格的浮点字面量：65 -> 65.f，0.15 -> 0.15f。"""
    text = f"{value:g}"
    if "e" in text or "E" in text:
        raise ValueError(f"不支持的数值格式: {value}")
    return text + "f" if "." in text else text + ".f"


def formula_cn(spec) -> str:
    parts = [f"{spec['base']:g}"]
    for attr, coef in spec["terms"]:
        parts.append(f"{ATTRS[attr]['cn']} × {coef:g}")
    if spec["level"]:
        parts.append(f"等级 × {spec['level']:g}")
    return " + ".join(parts)


def formula_cpp(spec, base_const) -> str:
    parts = [base_const]
    for attr, _ in spec["terms"]:
        parts.append(f"{ATTRS[attr]['var']} * {ATTRS[attr]['coef']}")
    if spec["level"]:
        parts.append("Level * LevelCoefficient")
    return " + ".join(parts)


def clamp_cn(spec) -> str:
    lo, hi = spec["lo"], spec["hi"]
    if hi is None:
        return f"最低 {lo:g}"
    return f"{lo:g} ~ {hi:g}"


def clamp_comment(spec) -> str:
    lo, hi = spec["lo"], spec["hi"]
    if hi is None:
        return f"// 兜底：结果不低于 {lo:g}"
    if lo == 0:
        return f"// 上限钳制：最高 {hi:g}"
    return f"// 区间钳制：{lo:g} ~ {hi:g}"


def render_header(class_key, stat):
    spec = stat["classes"][class_key]
    class_cn = next(c["cn"] for c in CLASSES if c["key"] == class_key)
    name = f"UMMC_{class_key}_{stat['key']}"
    return f"""// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ModMagnitudeCalculation/Common/AuraMMCBase.h"
#include "MMC_{class_key}_{stat['key']}.generated.h"

/**
 * {class_cn} · {stat['cn']}
 * 公式：{formula_cn(spec)}（钳制：{clamp_cn(spec)}）
 * {spec['note']}
 */
UCLASS()
class AURA_API {name} : public UAuraMMCBase
{{
	GENERATED_BODY()

public:
	{name}();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
}};
"""


def render_source(class_key, stat):
    spec = stat["classes"][class_key]
    class_cn = next(c["cn"] for c in CLASSES if c["key"] == class_key)
    name = f"UMMC_{class_key}_{stat['key']}"
    base_const = stat["base_const"]

    constants = [f"\tconstexpr float {base_const} = {cpp_float(spec['base'])};"]
    constants.append("")
    for attr, coef in spec["terms"]:
        constants.append(f"\tconstexpr float {ATTRS[attr]['coef']} = {cpp_float(coef)};")
    if spec["level"]:
        constants.append(f"\tconstexpr float LevelCoefficient = {cpp_float(spec['level'])};")
    constants_block = "\n".join(constants)

    if len(spec["terms"]) == 1:
        capture = f"\tCaptureTargetAttribute(UAuraAttributeSet::{ATTRS[spec['terms'][0][0]]['getter']}());"
    else:
        getters = ",\n\t\t".join(
            f"UAuraAttributeSet::{ATTRS[attr]['getter']}()" for attr, _ in spec["terms"]
        )
        capture = f"\tCaptureTargetAttributes({getters});"

    reads = [
        f"\tconst float {ATTRS[attr]['var']} = GetCapturedMagnitude(\n"
        f"\t\tUAuraAttributeSet::{ATTRS[attr]['getter']}(), Spec, EvaluationParameters);"
        for attr, _ in spec["terms"]
    ]
    if spec["level"]:
        reads.append("\tconst int32 Level = GetCharacterLevelFromSpec(Spec);")
    reads_block = "\n".join(reads)

    if spec["hi"] is None:
        result_expr = f"\treturn FMath::Max(Result, {cpp_float(spec['lo'])});"
    else:
        result_expr = f"\treturn FMath::Clamp(Result, {cpp_float(spec['lo'])}, {cpp_float(spec['hi'])});"

    # result_expr 之后还要闭合 CalculateBaseMagnitude 的函数体
    result_expr = result_expr + "\n}\n"

    return f"""// Study Game Ability System _Hades

#include "AbilitySystem/ModMagnitudeCalculation/{class_key}/MMC_{class_key}_{stat['key']}.h"
#include "AbilitySystem/AuraAttributeSet.h"

namespace
{{
	// {class_cn} · {stat['cn']}：{formula_cn(spec)}
	// {spec['note']}
{constants_block}
}}

{name}::{name}()
{{
{capture}
}}

float {name}::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{{
	const FAggregatorEvaluateParameters EvaluationParameters = BuildEvaluateParameters(Spec);

{reads_block}

	const float Result = {formula_cpp(spec, base_const)};

	{clamp_comment(spec)}
{result_expr}"""


def render_doc():
    lines = []
    add = lines.append

    add("# 派生属性自定义计算（MMC）—— 三职业重构")
    add("")
    add("> 所有派生属性（Derived Attributes）的 Magnitude 都由 C++ 的 MMC 计算，"
        "每个职业拥有独立的一套公式；修改数值只需改对应 `.cpp` 文件顶部的常量。")
    add("")
    add("## 1. 目录结构")
    add("")
    add("```text")
    add("Source/Aura/Public/AbilitySystem/ModMagnitudeCalculation/")
    add("├── Common/")
    add("│   └── AuraMMCBase.h                 // 公共基类：捕获、读取、等级解析")
    add("├── Elementalist/                     // 元素法师：14 个 MMC")
    add("├── Warrior/                          // 战士：14 个 MMC")
    add("├── Ranger/                           // 游侠：14 个 MMC")
    add("└── MMC_*.h                           // 旧版通用实现（仍被 GE_DerivedAttributes 引用）")
    add("```")
    add("")
    add("Private 目录保持与之完全对应的结构（`Private/AbilitySystem/ModMagnitudeCalculation/<职业>/`）。")
    add("")
    add("## 2. 公共基类 `UAuraMMCBase`")
    add("")
    add("| 能力 | 说明 |")
    add("| --- | --- |")
    add("| `CaptureTargetAttribute(s)` | 以「目标 + 非快照」方式登记捕获，派生属性随基础属性实时刷新 |")
    add("| `GetCapturedMagnitude` | 统一读取属性，未登记/读取失败返回 0，并保证结果非负 |")
    add("| `BuildEvaluateParameters` | 带上来源/目标标签，供属性聚合器做标签过滤 |")
    add("| `GetCharacterLevelFromSpec` | 先取 `SourceObject`，再退回 `Instigator`，解析失败返回 0 |")
    add("")
    add("子类只负责「捕获哪些属性 + 用什么公式」，不再重复样板代码。")
    add("")
    add("## 3. 职业定位")
    add("")
    add("| 职业 | 主属性 | 定位 | 强项 | 弱项 |")
    add("| --- | --- | --- | --- | --- |")
    for c in CLASSES:
        add(f"| {c['cn']} | {c['primary']} | {c['identity']} | {c['strengths']} | {c['weaknesses']} |")
    add("")
    add("### 属性由谁主导")
    add("")
    add("| 属性 | 元素法师 | 战士 | 游侠 |")
    add("| --- | --- | --- | --- |")
    add("| 最大生命值 | 最低 | **最高** | 中等 |")
    add("| 最大法力值 | **最高** | 最低 | 中等 |")
    add("| 物理攻击力 | 最低 | **最高** | 较高（敏捷） |")
    add("| 魔法攻击力 | **最高** | 最低 | 中等 |")
    add("| 护甲 | 最低 | **最高** | 较低 |")
    add("| 魔法抗性 | **最高** | 最低 | 中等 |")
    add("| 暴击几率 | 较低 | 中等 | **最高** |")
    add("| 命中几率 | 最低 | 较高 | **最高** |")
    add("| 闪避几率 | 最低 | 较低 | **最高** |")
    add("| 攻击速度 | 最低 | 中等 | **最高** |")
    add("| 冷却缩减 | **最高** | 最低 | 中等 |")
    add("| 生命恢复 | 最低 | **最高** | 中等 |")
    add("| 法力恢复 | **最高** | 最低 | 中等 |")
    add("| 移动速度 | 最低 | 中等 | **最高** |")
    add("")

    cn_map = {c["key"]: c["cn"] for c in CLASSES}
    for index, class_def in enumerate(CLASSES, start=1):
        key = class_def["key"]
        add(f"## 4.{index} {class_def['cn']}（{key}）")
        add("")
        add("| 属性 | 公式 | 钳制 | 策划说明 |")
        add("| --- | --- | --- | --- |")
        for stat in STATS:
            spec = stat["classes"][key]
            add(f"| {stat['cn']} | {formula_cn(spec)} | {clamp_cn(spec)} | {spec['note']} |")
        add("")

    add("## 5. 接入 GameplayEffect")
    add("")
    add("1. 编译 C++（UHT 会为每个职业生成 14 个 `UMMC_<职业>_<属性>` 类）。")
    add("2. 打开敌人/玩家的派生属性 GE：")
    add("   - `Content/Blueprint/AbilitySystem/GameplayEffects/DefaultAttributes/Enemy/Elementalist/GE_DerivedAttributes_Elementalist`")
    add("   - `.../Warrior/GE_DerivedAttributes_Warrior`")
    add("   - `.../Ranger/GE_DerivedAttributes_Ranger`")
    add("3. 对 14 个 Modifier 逐个设置：")
    add("   - **Modifier Op**：`Add`（与旧实现一致，MMC 返回的是最终值）。")
    add("   - **Magnitude Calculation Type**：`Custom Calculation Class`。")
    add("   - **Calculation Class**：选对应职业的 MMC，例如元素法师的 `MaxHealth` 选 `MMC_Elementalist_MaxHealth`。")
    add("4. 阵营通用 GE `DefaultAttributes/Aura/GE_DerivedAttributes` 仍使用旧的通用 MMC，"
        "如需玩家角色也走职业化公式，把它的 14 个 Modifier 重指到对应职业即可。")
    add("5. 施加顺序由 `AAuraCharacterBase::InitDefaultAttributes` 保证："
        "`PrimaryAttributes` → `DerivedAttributes` → `PoolAttributes`；"
        "`UAuraAttributeSet::PostGameplayEffectExecute` 会在 `MaxHealth/MaxMana` 变化时重新钳制 `Health/Mana`。")
    add("")
    add("## 6. 调参与扩展")
    add("")
    add("- **改数值**：直接修改对应 `.cpp` 顶部的 `constexpr` 常量（基础值 / 各属性系数 / 等级系数），无需改公式。")
    add("- **新增属性**：在 `STATS` 表中加一条，运行 `python Tools/ModMagnitudeCalculation/generate_mmc.py` "
        "会补齐三职业的头文件与源文件；也可以在三个职业目录下手写一份同构文件。")
    add("- **新增职业**：在 `CLASSES` / `STATS` 中补一个职业键，生成器会按同样结构输出新目录。")
    add("- **概率类属性**：钳制写在 MMC 内部，保证 GE 的加法叠加不会突破上限；"
        "如果后续还要通过 Buff 继续加成，需要把钳制下沉到 `UAuraAttributeSet::PreAttributeChange`。")
    add("")
    add("## 7. 关于旧文件")
    add("")
    add("`ModMagnitudeCalculation/MMC_*.h`（14 个）是重构前的通用实现，"
        "仍被 `DefaultAttributes/Aura/GE_DerivedAttributes` 引用，因此本次 **保留未动**；"
        "三职业 GE 全部迁移到新类之后，可以再统一删除。")
    add("")
    return "\n".join(lines)


# ============================================================================
# 主流程
# ============================================================================
def validate():
    class_keys = [c["key"] for c in CLASSES]
    seen = set()
    for stat in STATS:
        if stat["key"] in seen:
            raise ValueError(f"重复的属性键: {stat['key']}")
        seen.add(stat["key"])
        for key in class_keys:
            if key not in stat["classes"]:
                raise ValueError(f"{stat['key']} 缺少职业 {key}")
            for attr, _ in stat["classes"][key]["terms"]:
                if attr not in ATTRS:
                    raise ValueError(f"{stat['key']} 引用了未知属性 {attr}")


def main():
    validate()

    written = 0
    for class_def in CLASSES:
        class_key = class_def["key"]
        public_dir = PUBLIC_MMC / class_key
        private_dir = PRIVATE_MMC / class_key
        public_dir.mkdir(parents=True, exist_ok=True)
        private_dir.mkdir(parents=True, exist_ok=True)

        for stat in STATS:
            header_path = public_dir / f"MMC_{class_key}_{stat['key']}.h"
            source_path = private_dir / f"MMC_{class_key}_{stat['key']}.cpp"

            header_path.write_text(render_header(class_key, stat), encoding="utf-8")
            source_path.write_text(render_source(class_key, stat), encoding="utf-8")
            written += 2

    DOC_PATH.parent.mkdir(parents=True, exist_ok=True)
    DOC_PATH.write_text(render_doc(), encoding="utf-8")

    print(f"已生成 {written} 个文件（{len(CLASSES)} 个职业 × {len(STATS)} 个属性）")
    print(f"文档: {DOC_PATH}")


if __name__ == "__main__":
    main()
