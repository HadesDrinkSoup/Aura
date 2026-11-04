# 职业（Character Class）函数调用

_属性 GE · 启动技能 · 数值公式 · AURA / UE5 GAS_

## ① 数据结构 — 职业存在哪

- `ECharacterClass`：Elementalist / Warrior / Ranger / Tank
- `FCharacterClassDefaultInfo`：3 个属性 GE + 启动技能
- `UCharacterClassInfo::FindClassDefaultInfo()`：TMap 查找
- `UPlayerCharacterDefinition::FindClassDefaultInfo()`：展示层查表
- `UAuraCharacterRoster::GetAllCharacters()`：按 SortOrder 排序

## ② 读取配置 — 唯一的取数路径

- `UAuraGameSettings::Get()`：GetMutableDefault，进程级
- `GetCharacterClassInfo()`：LoadSynchronous + 未配置告警
- `UAuraAbilitySystemLibrary::GetCharacterClassInfo()`：唯一入口
- `GetCharacterRoster()`：选人名单，可为空
- DefaultGame.ini → `DA_CharacterClassInfo`

## ③ 属性初始化链 — InitDefaultAttributes 顺序

- `PossessedBy()`（玩家）/ `BeginPlay()`（敌人）
- → `InitActorInfo()` → `InitDefaultAttributes()`
- `ASC->InitAbilityActorInfo(Owner, Avatar)`
- `HasAuthority` + ASC 判空 → `FindClassDefaultInfo()`
- 优先级：BP 显式 GE ＞ 职业表 GE
- `MakeEffectContext()` + `AddInstigator` / `AddSourceObject`
- `Level = Max(GetCharacterLevel(), 1)` → Spec 等级
- `ApplyEffectToSelf(Primary)`：主属性
- 移除旧 Derived → `ApplyEffectToSelf(Derived)`：Infinite
- `AdditionalInitialEffects`：精英 / 装备 / 诅咒
- `ApplyEffectToSelf(Pool)`：最后，填满 Health / Mana
- `MakeOutgoingSpec()` → `ApplyGameplayEffectSpecToSelf()`

## ④ 技能授予 — 职业技能 + BP 技能

- `AddCharacterAbilities()`：Authority 检查
- `GetCombinedStartupAbilities()`：BP + 职业，AddUnique
- `ASC->AddCharacterAbilities()`：遍历并 `GiveAbility`
- `AbilityInputTag` → DynamicSpecSourceTags（输入匹配）

## ⑤ 选人 → 出生 — SetCharacterClass 的时机

- `SetSelectedCharacter(Def)`：选人界面写入
- `GetSelectedPawnClass()`：`PawnClass.LoadSynchronous()`
- `GetSelectedCharacterClass(Out)`：`Definition->CharacterClass`
- `GetDefaultPawnClassForController()`：选中的 Pawn 类
- `SpawnDefaultPawnAtTransform()` → `SetCharacterClass()`
- 时序：必须早于 `PossessedBy()`，否则按错职业下发
- `OpenLevel` 后 GameMode 重建，选择留在 Subsystem

## ⑥ 数值公式 · MMC — 每个职业 14 个 MMC

- `UAuraMMCBase::CaptureTargetAttribute(s)`：目标 + 非快照
- `GetCapturedMagnitude()` / `BuildEvaluateParameters()`
- `GetCharacterLevelFromSpec()`：SourceObject → Source
- 每职业 14 个 `MMC_<Class>_<Attr>`，系数 `static constexpr`
- 主属性：Strength / Intelligence / Agility / Vigor
- 14 项派生属性明细见 Docs/ModMagnitudeCalculation.md

## ⑦ 约定与坑 — 改代码前先看

- TMap 无序 → 选人用 Roster + SortOrder
- BP 覆盖是整套替换，不逐 Modifier 合并
- Derived 必须 Infinite；重初始化先移除旧实例
- Instant 的 Multiply 乘在 base 0 上 → 像没生效
- 客户端 `GetGameMode()` 为 null → 配置放 Settings
- 新增职业改 C++：枚举 + 14 MMC + 3 GE + 表一条
