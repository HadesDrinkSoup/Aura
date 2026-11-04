# -*- coding: utf-8 -*-
"""把 _mage_damage_matrix.json 渲染成 Docs/Elementalist_Damage_Matrix.md。"""
import json
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "Docs", "_mage_damage_matrix.json")
DST = os.path.join(ROOT, "Docs", "Elementalist_Damage_Matrix.md")

CLASS_CN = {"Elementalist": "元素法师", "Warrior": "战士", "Ranger": "游侠", "Tank": "坦克"}
ORDER = ["Elementalist", "Warrior", "Ranger", "Tank"]
LEVELS = [1, 5, 10, 15, 20, 50]


def fmt(v, nd=2):
    s = f"{v:.{nd}f}"
    if "." in s:
        s = s.rstrip("0").rstrip(".")
    return s


def table(headers, rows):
    out = ["| " + " | ".join(headers) + " |",
           "| " + " | ".join(["---"] * len(headers)) + " |"]
    for r in rows:
        out.append("| " + " | ".join(str(c) for c in r) + " |")
    return "\n".join(out)


def main():
    rows = json.load(open(SRC, encoding="utf-8"))
    idx = {(r["level"], r["target"]): r for r in rows}

    # ---- 结论速览 ----
    quick = []
    for lv in (1, 50):
        for c in ORDER:
            r = idx[(lv, c)]
            quick.append([f"Lv.{lv}", CLASS_CN[c], fmt(r["mr"], 1), str(r["hp"]),
                          f"{fmt(r['mit_pct'])}%", fmt(r["final"]), str(r["hits"])])
    t_quick = table(["等级", "目标职业", "魔抗", "生命", "减伤率", "火球单发", "击杀发数"], quick)

    # ---- 输入表 ----
    t_in = table(["等级"] + [CLASS_CN[c] for c in ORDER] + ["原始伤害 Raw"],
                 [[f"Lv.{lv}"] + [f"{fmt(idx[(lv, c)]['mr'], 1)} / {idx[(lv, c)]['hp']}" for c in ORDER]
                  + [fmt(idx[(lv, ORDER[0])]["raw_mag"])] for lv in LEVELS])

    # ---- 减伤率 ----
    t_mit = table(["等级", "K"] + [CLASS_CN[c] for c in ORDER],
                  [[f"Lv.{lv}", str(idx[(lv, ORDER[0])]["k"])]
                   + [f"{fmt(idx[(lv, c)]['mit_pct'])}%" for c in ORDER] for lv in LEVELS])

    # ---- 最终伤害 ----
    t_final = table(["等级", "原始伤害"] + [CLASS_CN[c] for c in ORDER],
                    [[f"Lv.{lv}", fmt(idx[(lv, ORDER[0])]["raw_mag"])]
                     + [fmt(idx[(lv, c)]["final"]) for c in ORDER] for lv in LEVELS])

    # ---- 击杀发数 ----
    t_hits = table(["等级"] + [CLASS_CN[c] for c in ORDER],
                   [[f"Lv.{lv}"] + [str(idx[(lv, c)]["hits"]) for c in ORDER] for lv in LEVELS])

    # ---- 物理参考 ----
    t_phy = table(["等级"] + [CLASS_CN[c] for c in ORDER],
                  [[f"Lv.{lv}"] + [f"{fmt(idx[(lv, c)]['phy_final'])}（{idx[(lv, c)]['phy_hits']} 发）"
                                   for c in ORDER] for lv in LEVELS])

    # ---- 排序 ----
    rank1 = " ＞ ".join(f"{CLASS_CN[c]}({fmt(idx[(1, c)]['final'])})" for c in sorted(ORDER, key=lambda c: -idx[(1, c)]["final"]))
    rank50 = " ＞ ".join(f"{CLASS_CN[c]}({fmt(idx[(50, c)]['final'])})" for c in sorted(ORDER, key=lambda c: -idx[(50, c)]["final"]))
    hits1 = " < ".join(f"{CLASS_CN[c]}({idx[(1, c)]['hits']})" for c in sorted(ORDER, key=lambda c: idx[(1, c)]["hits"]))
    hits50 = " < ".join(f"{CLASS_CN[c]}({idx[(50, c)]['hits']})" for c in sorted(ORDER, key=lambda c: idx[(50, c)]["hits"]))

    doc = f"""# 元素法师对各职业伤害矩阵（按项目 DataTable 实算）

> 目标：只看一件事——同一个元素法师，把火球丢到四个职业身上，各吃多少伤害、要几发打死。
> 全部数值来自 `DataTable/` 下的真实 CSV 与各职业 MMC 公式，未使用任何示例数值。

## ① 一句话结论

同一发火球，Lv.1 打元素法师 {fmt(idx[(1,'Elementalist')]['final'])} 点、打坦克 {fmt(idx[(1,'Tank')]['final'])} 点，差 {fmt(idx[(1,'Elementalist')]['final'] / idx[(1,'Tank')]['final'])} 倍；
但击杀发数从 {idx[(1,'Elementalist')]['hits']} 发拉大到 {idx[(1,'Tank')]['hits']} 发，**{idx[(1,'Tank')]['hits'] / idx[(1,'Elementalist')]['hits']:.1f} 倍**——差距主要来自血量，魔抗只是放大器。

{t_quick}

## ② 计算口径

**攻击侧（元素法师，均为该等级下的确定值）**

```text
魔法攻击 MagicalAttack = 12 + 智力 × 3.4 + 等级 × 1.5
火球基础伤害           = CT_Damage.csv 中 Abilities.FireBall 对应等级列
原始伤害 Raw           = 火球基础伤害 + MagicalAttack × AttackScaling(0.6)
```

**防御侧（目标职业）**

```text
K      = DefenseBase(50) + DefensePerLevel(10) × (施法者等级 − 1)
减伤率 = 魔抗 / (魔抗 + K)
最终伤害 = Raw × (1 − 减伤率)，下限 Raw × MinDamageRatio(0.1)
击杀发数 = ceil(目标最大生命 / 最终伤害)
```

| 参数 | 值 | 位置 |
| --- | --- | --- |
| AttackScaling | 0.6 | `ExecutionCalculation_Damage.h` |
| DefenseBase | 50 | 同上 |
| DefensePerLevel | 10 | 同上 |
| MinDamageRatio | 0.1 | 同上 |
| 技能等级 | 等于施法者角色等级 | `UAuraGameplayAbility::SetAbilityLevel` |

魔抗与生命来自各职业 MMC（`Source/Aura/Public/AbilitySystem/ModMagnitudeCalculation/<职业>/`）：

| 职业 | 魔抗公式 | 生命公式 |
| --- | --- | --- |
| 元素法师 | 3 + 活力×0.45 + 智力×0.35 + L×0.4 | 65 + 活力×7 + 力量×2 + L×7 |
| 战士 | 1 + 活力×1.6 + 智力×0.15 + L×1.1 | 140 + 活力×15 + 力量×9 + L×15 |
| 游侠 | 2 + 活力×0.5 + 智力×0.5 + L×0.25 | 90 + 活力×9 + 力量×4 + L×9 |
| 坦克 | 4 + 活力×1.4 + 智力×0.3 + L×1.4 | 200 + 活力×18 + 力量×9 + L×16 |

## ③ 输入：各目标在该等级的「魔抗 / 最大生命」

同一等级下法师的原始伤害是固定的，列在最后一列方便对照。

{t_in}

## ④ 减伤率：K 在涨，魔抗涨得更快

{t_mit}

读法：K 按固定斜率 +10/级上升，各职业魔抗按自身系数上升。法师魔抗成长最慢（0.45 活力 + 0.35 智力），坦克最快（1.4 活力 + 1.4 等级），所以减伤率在高等级仍然被坦克牢牢占住，战士紧随其后。

## ⑤ 火球单发最终伤害

{t_final}

- Lv.1 单发伤害排序（高 → 低）：{rank1}
- Lv.50 单发伤害排序（高 → 低）：{rank50}

## ⑥ 击杀发数（ceil(生命 / 单发伤害)）

{t_hits}

- Lv.1 效率排序：{hits1}
- Lv.50 效率排序：{hits50}

## ⑦ 成长曲线速览

```text
最终伤害（对四职业，Lv.1 → Lv.50）
坦克    39.04 ──────────────────────────────────────────── 380.77   ×9.75
战士    43.66 ───────────────────────────────────────────── 407.85  ×9.34
元素    49.51 ───────────────────────────────────────────── 460.28  ×9.30
游侠    52.98 ───────────────────────────────────────────── 477.73  ×9.02
```

伤害倍数约 9 倍，而 Lv.1→Lv.50 的火球基础伤害是 20→300（15 倍）：**被削掉的部分正是减伤率在前中期维持 10~38%、后期仍有 8~27% 的结果**，属性成长没有跑赢 K 的增长。

## ⑧ 物理普攻参考（未配基础伤害）

`CT_Damage.csv` 目前只配置了 `Abilities.FireBall`，普攻没有基础伤害行，因此下表是「仅 物攻×0.6」的下限值，只用于判断量级，不能当实际普攻伤害。

{t_phy}

法师的物理攻击 = 3 + 力量×1 + 敏捷×0.3 + L×0.6，系数极低，所以物理路线在四职业里垫底是设计使然，不是数值 bug。

## ⑨ 能读出的三件事

1. **减伤率排序 = 血量排序**，四职业没有出现「高魔抗低血」的错位，说明 MMC 系数是同向单调的：坦克两头都占，游侠两头都薄。这让「选职业」在 PvP 里等价于「选难度」。
2. **元素法师打元素法师 2~3 发就能秒**（Lv.15 起 2 发），内战节奏远快于外战。若后续加 PvP，这是最需要单独调的一条。
3. **K 的增长偏快**：Lv.50 时 K=540，而坦克魔抗 199.6，减伤率仍 27%。如果设计意图是「后期减伤率应快速衰减、让伤害膨胀」，应下调 `DefensePerLevel`；若意图是「后期靠血量扛」，则当前曲线成立，但击杀发数会在 8~16 之间平台化（已可见：Lv.20→Lv.50 只从 9 发降到 8 发）。

## ⑩ 复现方式

```powershell
& "C:\\Users\\20770\\.dsh\\dsh-runtimes\\dsh-primary-runtime\\dependencies\\python\\python.exe" "Docs\\_calc_mage_damage.py"
```

脚本直接解析 `DataTable/*.csv` 并复刻 MMC 公式，改完表重跑即可，无需编译。

## ⑪ 未接入的机制（因此上表是确定值）

- 暴击：无
- 命中 / 闪避：无
- 防御穿透：无
- 物理与魔法分离的 K：两段共用同一套 DefenseBase / DefensePerLevel
- 属性易伤 / 增伤：无

详见 `Docs/DamageCalculation_Formula.md` ⑤ 节。对照说明：那份文档里「Lv.1 火球 → 坦克 = 35.9」用的是示例基础伤害 15，本文档用 `CT_Damage.csv` 的真实值 20，故为 {fmt(idx[(1,'Tank')]['final'])}。
"""
    with open(DST, "w", encoding="utf-8", newline="\n") as f:
        f.write(doc)
    print("wrote", DST, len(doc), "chars")


if __name__ == "__main__":
    main()
