# -*- coding: utf-8 -*-
"""从项目 DataTable + MMC 公式计算元素法师对各职业的伤害矩阵（一次性的校验脚本）。"""
import csv
import json
import math
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DT = os.path.join(ROOT, "DataTable")
LEVELS = [1, 5, 10, 15, 20, 50]

PRIMARY = {
    "Elementalist": "CT_PrimaryAttributes_Elementalist.csv",
    "Warrior": "CT_PrimaryAttributes_Warrior.csv",
    "Ranger": "CT_PrimaryAttributes_Ranger.csv",
    "Tank": "CT_PrimaryAttributes_Tank.csv",
}


def read_primary(fn):
    path = os.path.join(DT, fn)
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    levels = [int(float(v)) for v in rows[0][1:]]
    out = {}
    for r in rows[1:]:
        if not r or not r[0].strip():
            continue
        out[r[0].strip().split(".")[-1]] = [float(v) for v in r[1:]]
    return levels, out


def read_base_damage():
    path = os.path.join(DT, "CT_Damage.csv")
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(f))
    levels = [int(float(v)) for v in rows[0][1:]]
    for r in rows[1:]:
        if r[0].startswith("Abilities.FireBall"):
            return levels, [float(v) for v in r[1:]]
    raise SystemExit("FireBall 行未找到")


# ---- MMC 公式（与 Source/Aura/Public/AbilitySystem/ModMagnitudeCalculation 下各头文件一致）----
def magical_attack(str_, int_, agi, vig, lv):
    return 12 + int_ * 3.4 + lv * 1.5


def physical_attack(str_, int_, agi, vig, lv):
    return 3 + str_ * 1 + agi * 0.3 + lv * 0.6


def armor(cls, str_, int_, agi, vig, lv):
    return {
        "Elementalist": 0 + vig * 0.8 + str_ * 0.2 + lv * 0.15,
        "Warrior": 5 + vig * 1.8 + str_ * 1.3 + lv * 0.55,
        "Ranger": 1 + vig * 1 + str_ * 0.35 + lv * 0.25,
        "Tank": 8 + vig * 2.6 + str_ * 1.4 + lv * 0.7,
    }[cls]


def magic_resistance(cls, str_, int_, agi, vig, lv):
    return {
        "Elementalist": 3 + vig * 0.45 + int_ * 0.35 + lv * 0.4,
        "Warrior": 1 + vig * 1.6 + int_ * 0.15 + lv * 1.1,
        "Ranger": 2 + vig * 0.5 + int_ * 0.5 + lv * 0.25,
        "Tank": 4 + vig * 1.4 + int_ * 0.3 + lv * 1.4,
    }[cls]


def max_health(cls, str_, int_, agi, vig, lv):
    return {
        "Elementalist": 65 + vig * 7 + str_ * 2 + lv * 7,
        "Warrior": 140 + vig * 15 + str_ * 9 + lv * 15,
        "Ranger": 90 + vig * 9 + str_ * 4 + lv * 9,
        "Tank": 200 + vig * 18 + str_ * 9 + lv * 16,
    }[cls]


ATTACK_SCALING = 0.6
DEFENSE_BASE = 50.0
DEFENSE_PER_LEVEL = 10.0
MIN_DAMAGE_RATIO = 0.1


def mitigate(raw, defense, caster_level):
    k = DEFENSE_BASE + DEFENSE_PER_LEVEL * (caster_level - 1)
    m = defense / (defense + k)
    final = raw * (1 - m)
    floor = raw * MIN_DAMAGE_RATIO
    return k, m, max(final, floor)


def main():
    data = {c: read_primary(fn) for c, fn in PRIMARY.items()}
    fb_levels, fb = read_base_damage()
    rows = []
    for i, lv in enumerate(LEVELS):
        mage = data["Elementalist"][1]
        ma = magical_attack(mage["Strength"][i], mage["Intelligence"][i],
                            mage["Agility"][i], mage["Vigor"][i], lv)
        pa = physical_attack(mage["Strength"][i], mage["Intelligence"][i],
                             mage["Agility"][i], mage["Vigor"][i], lv)
        base = fb[i]
        raw_mag = base + ma * ATTACK_SCALING
        raw_phy = pa * ATTACK_SCALING  # 只有普攻基线时的下限（CT_Damage 未配普攻基础伤害）
        for cls in ["Elementalist", "Warrior", "Ranger", "Tank"]:
            _, p = data[cls]
            s, it, a, v = p["Strength"][i], p["Intelligence"][i], p["Agility"][i], p["Vigor"][i]
            mr = magic_resistance(cls, s, it, a, v, lv)
            ar = armor(cls, s, it, a, v, lv)
            hp = max_health(cls, s, it, a, v, lv)
            k, m, final = mitigate(raw_mag, mr, lv)
            _, m2, final_p = mitigate(raw_phy, ar, lv)
            rows.append({
                "level": lv, "target": cls,
                "mage_ma": round(ma, 1), "mage_pa": round(pa, 1), "base": base,
                "raw_mag": round(raw_mag, 2), "mr": round(mr, 1), "k": int(k),
                "mit_pct": round(m * 100, 2), "final": round(final, 2),
                "hp": int(round(hp)), "hits": math.ceil(hp / final),
                "remain_pct": round((hp - final) / hp * 100, 1),
                "armor": round(ar, 1), "raw_phy": round(raw_phy, 2),
                "phy_mit_pct": round(m2 * 100, 2), "phy_final": round(final_p, 2),
                "phy_hits": math.ceil(hp / final_p),
            })
    out = os.path.join(ROOT, "Docs", "_mage_damage_matrix.json")
    with open(out, "w", encoding="utf-8") as f:
        json.dump(rows, f, ensure_ascii=False, indent=1)
    print("wrote", out, len(rows), "rows")
    for r in rows:
        print(f"Lv{r['level']:>2} {r['target']:<13} MR{r['mr']:>6} HP{r['hp']:>5} "
              f"raw{r['raw_mag']:>7} mit{r['mit_pct']:>6}% final{r['final']:>7} hits{r['hits']:>3} "
              f"| phy {r['phy_final']:>7} hits{r['phy_hits']:>3}")


if __name__ == "__main__":
    main()
