# 职业配置表（CharacterClassInfo）—— 让敌人与职业解耦

> 目标：**角色 BP 只声明「我是哪个职业、几级」**，属性 GE 与默认技能统一从数据资产下发。
> 新增一个敌人 = 新建一个 BP + 选职业 + 填等级，不再复制/拖拽 3 个 GE。
>
> 本表只描述**数值**，敌我共用；「可选角色 / 选人界面」的展示层见 [CharacterSelection.md](CharacterSelection.md)。

## 1. 数据流

```text
Project Settings → Game → Aura Game Settings   (UAuraGameSettings)
└── CharacterClassInfo ──────────────┐
                                     ▼
                       DA_CharacterClassInfo (UCharacterClassInfo)
                       ├── TMap<ECharacterClass, FCharacterClassDefaultInfo>
                       │     ├── Elementalist → Primary / Derived / Pool GE + StartupAbilities
                       │     ├── Warrior      → ...
                       │     ├── Ranger       → ...
                       │     └── Tank         → ...
                       └── CommonAbilities                  ← 全职业共享的起始技能

Bp_AuraCharacter / Bp_Goblin_* (AAuraCharacterBase)
├── CharacterClass = Elementalist / Warrior / Ranger / Tank   ← 唯一的职业入口
├── Level（敌人可编辑）                                ← 影响 GE 等级与 MMC 等级系数
├── Primary/Derived/PoolAttributes                     ← 可选：个体覆盖（一般留空）
└── AdditionalInitialEffects                           ← 个体差异（精英/首领）
```

初始化时 `AAuraCharacterBase::InitDefaultAttributes`：

1. 通过 `UAuraAbilitySystemLibrary::GetCharacterClassInfo` 读 Project Settings 里的 `DA_CharacterClassInfo`
2. 按 `CharacterClass` 查 `FCharacterClassDefaultInfo`
3. 依次施加 `Primary → Derived → AdditionalInitialEffects → Pool`
   （个体 GE 放在 Pool 之前，Pool 才能用最终的 MaxHealth/MaxMana 把生命/法力填满）
4. 派生的技能：`StartupAbilities`（BP）+ 职业的 `StartupAbilities` + 表的 `CommonAbilities`
   取并集去重后一起授予（顺序 BP → 职业 → 通用，见 `AAuraCharacterBase::GetCombinedStartupAbilities`）

> 玩家角色的 `CharacterClass` 可以在运行时被覆盖：战斗关的 `AAuraGameModeBase` 会在
> spawn 之后、`PossessedBy` 之前，按选人结果调用 `SetCharacterClass()`。

## 2. 解析优先级

| 位置 | 用途 |
| --- | --- |
| BP 上的 `PrimaryAttributes / DerivedAttributes / PoolAttributes` | **显式覆盖**：不为空时优先使用，用于职业表之外的极端个体 |
| `DA_CharacterClassInfo` 中对应职业的 3 个 GE | 默认来源，绝大多数角色用这条 |
| `AdditionalInitialEffects` | 在 Derived **之后、Pool 之前**追加，适合「精英 +属性」「+50% 生命」「被诅咒 -20% 移速」。注意：要让百分比生效，GE 必须是 **Infinite**（Instant 的 Multiply 只会作用在 base 0 上） |

> 覆盖是"整套替换"而不是逐个 Modifier 合并：一旦 BP 上填了 Derived GE，就不会再用职业表里的那个。

GE 等级统一取 `GetCharacterLevel()`（最低 1），所以主要属性 GE 用 CurveTable 时可以直接按等级取数值。
三条职业的主属性曲线表（按等级的 力量/智力/敏捷/活力）见 [PrimaryAttributes_CurveTable.md](PrimaryAttributes_CurveTable.md)。

## 3. 新增一个敌人（不改 C++）

1. 复制一个现有敌人 BP（例如 `Bp_Goblin_Spear`），父类仍是 `Bp_AuraEnemyBase`。
2. Details → `Character Class` → 选 `Warrior / Ranger / Elementalist / Tank`。
3. `Level` 填等级（1、5、10……）。
4. 需要个体差异时，在 `Additional Initial Effects` 里加修正 GE（可以不建新职业）。
5. 保证 BP 上三个 `...Attributes` 覆盖字段为空。

## 4. 新增一个职业（需要 C++）

属性公式在 C++ 的 MMC 里，因此新职业 = 新 MMC + 新 GE + 表里加一条：

1. **MMC**：复制 `ModMagnitudeCalculation/<某职业>/` 下 14 个文件，重命名为 `MMC_<新职业>_<属性>`，改类名、`CaptureTargetAttributes(...)` 与 `static constexpr` 系数。
2. **枚举**：`ECharacterClass` 里加一个值（例如 `Paladin`）。
3. **GE**：复制一个职业的 `GE_Primary_Attributes_* / GE_DerivedAttributes_* / GE_Pool_Attributes_*`，把 14 个 Modifier 的 Calculation Class 指到新 MMC；`Derived` 必须是 **Infinite**，`Pool` 的 Backing Attribute 必须是 **Target**。
4. **数据**：`DA_CharacterClassInfo` → `Character Class Information` 加一条 `Paladin → 3 个 GE`，保存。
5. 角色 BP 选新职业即可；不需要重新接线任何 BP。

## 5. 为什么放在 Project Settings（而不是 GameMode）

- **与关卡解耦**：数据不属于某个关卡。菜单关 / 战斗关是两张地图、两个 GameMode，放 GameMode 就得配两遍。
- **客户端可读**：`UGameplayStatics::GetGameMode()` 在客户端返回 `nullptr`，放在 GameMode 上，客户端 UI（选人界面）永远读不到职业表；`UDeveloperSettings` 是配置对象，哪里都能读。
- **单一数据源**：职业数值只在一处维护，改一个 GE 全职业生效；`AAuraGameModeBase` 也因此只剩「创建 Pawn」一件事。
- 读取路径统一收在 `UAuraAbilitySystemLibrary::GetCharacterClassInfo`，其它代码不直接依赖配置类。

## 6. 当前项目的接线状态

| 项 | 状态 |
| --- | --- |
| `DA_CharacterClassInfo` | 已填入 Elementalist / Warrior / Ranger 三套 GE；`Tank` 待登记（见下一行） |
| `Tank` 职业 | C++（枚举 + 14 个 `MMC_Tank_*`）与 `DataTable/CT_PrimaryAttributes_Tank.csv` 已就绪；还需在编辑器建 `CT_PrimaryAttributes_Tank` / 3 个 `GE_*_Tank` 并登记到 `DA_CharacterClassInfo` |
| `DefaultGame.ini` → `[/Script/Aura.AuraGameSettings]` | 已指向 `DA_CharacterClassInfo` |
| `Bp_AuraGameMode.CharacterClassInfo` | 该属性已从 `AAuraGameModeBase` 删除；BP 里残留的旧值会被忽略 |
| `Bp_AuraCharacter` | `Elementalist`，BP 上的 GE 覆盖已清空 |
| `Bp_Goblin_Slingshot` | `Ranger`，覆盖已清空 |
| `Bp_Goblin_Spear` | `Warrior`，覆盖已清空 |
| 运行时验证（无头启动） | 玩家 `MaxHealth 106`、Slingshot `141`、Spear `302`，无 GE 报错 |

## 7. 可继续扩展的方向

- **物种与职业分离**：职业只管属性；再建一个 `UEnemyDefinition`（Mesh、动画、武器、技能、掉落、经验），敌人 BP 引用它，避免每个哥布林 BP 重复配资源。
- **纯数据的职业变体**：若希望「GoblinWarrior」这种变体不改 C++，可以把 `TMap` 的键从 `ECharacterClass` 换成 `FGameplayTag`（如 `Character.Class.Warrior.Goblin`），MMC 仍复用 Warrior 的那一套。代价是失去枚举的编译期检查。
- **难度/精英倍率**：用一个对 `MaxHealth / PhysicalAttack` 做 `Multiply` 的 **Infinite** GE 放进 `AdditionalInitialEffects`，比复制整套 GE 更省事。
  （派生属性是由 Infinite GE 以 `Add` 叠加的，Instant 的 Multiply 只会乘 base=0，看起来就是“没生效”。）
- **技能也进表**：`FCharacterClassDefaultInfo` 已经带 `StartupAbilities`，`UCharacterClassInfo` 上的
  `CommonAbilities`（全职业共享技能）也已接通；后续可再加 `PassiveEffects`（出生即施加的被动 GE）等字段。
