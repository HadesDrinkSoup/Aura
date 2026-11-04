# 选人架构（Character Selection）—— 配置、状态、出生三层分离

> 选人关和战斗关是**两个独立关卡**（`OpenLevel` 硬切换），所以：
> 配置表不能挂在 GameMode 上（换关卡就没了，客户端也读不到），
> 选人结果也不能存在 GameMode 里（关卡一切换，GameMode 实例就重建了）。

## 1. 三层职责

| 层 | 类型 | 生命周期 | 谁读 |
| --- | --- | --- | --- |
| 全局配置 | `UAuraGameSettings`（`UDeveloperSettings`）+ `DA_CharacterClassInfo` / `DA_CharacterRoster` | 进程级 | 服务端 + 客户端 + 编辑器 |
| 本局选择 | `UAuraCharacterSelectionSubsystem`（`UGameInstanceSubsystem`） | 跨关卡（GameInstance 存活期间） | 战斗关 GameMode |
| 出生 | `AAuraGameModeBase::GetDefaultPawnClassForController` / `SpawnDefaultPawnAtTransform` | 单关卡 | 服务端 |

对比 Lyra：`ULyraPawnData`（每个角色一个 PrimaryDataAsset，AssetManager 注册）≈ `UPlayerCharacterDefinition`；
本局选中的 PawnData 存在 `PlayerState` 上（replicated）≈ 本项目的 `UAuraCharacterSelectionSubsystem`
（单机/本地多人够用；联机见第 5 节）。

## 2. 文件与职责

| 文件 | 职责 |
| --- | --- |
| `Source/Aura/Public/Game/AuraGameSettings.h` | Project Settings → Game → Aura Game Settings；持有 `DA_CharacterClassInfo`、`DA_CharacterRoster` 两个软引用 |
| `Source/Aura/Public/Game/PlayerCharacterDefinition.h` | 一个**可玩角色**：显示名 / 介绍 / 立绘 / `PawnClass` / 指向的 `ECharacterClass` / 排序 / 是否默认解锁 |
| `Source/Aura/Public/Game/AuraCharacterRoster.h` | 可选角色名单；`GetAllCharacters()` 按 `SortOrder` 排好序 |
| `Source/Aura/Public/Game/AuraCharacterSelectionSubsystem.h` | 保存本局选择；给 GameMode 提供 `GetSelectedPawnClass()` / `GetSelectedCharacterClass()` |
| `Source/Aura/Public/Game/AuraGameModeBase.h` | 只负责：按选择决定 Pawn 类 + spawn 后写 `CharacterClass` |
| `Source/Aura/Public/AbilitySystem/Data/CharacterClassInfo.h` | 数值层（属性 GE + 职业技能），敌我共用，**没有**展示字段 |

## 3. 数据流

```text
[选人关]  Widget（未实现，后续做）
   │  遍历 UAuraCharacterRoster::GetAllCharacters()
   │  预览：TSoftClassPtr<AAuraCharacterBase> PawnClass 加载后 spawn 一个站桩 Actor
   ▼
UAuraCharacterSelectionSubsystem::SetSelectedCharacter(Definition)
   │
   │  OpenLevel("/Game/Maps/StartMap")
   ▼
[战斗关] AAuraGameModeBase
   ├── GetDefaultPawnClassForController_Implementation → Definition->PawnClass.LoadSynchronous()
   ├── SpawnDefaultPawnAtTransform_Implementation → AuraPawn->SetCharacterClass(Definition->CharacterClass)
   ▼
AAuraCharacter::PossessedBy → InitActorInfo → InitDefaultAttributes
   └── 按 CharacterClass 查 DA_CharacterClassInfo → 发 Primary / Derived / Pool GE
```

**时序很关键**：`SetCharacterClass` 必须在 `PossessedBy` 之前完成。`SpawnDefaultPawnAtTransform` 正好在
`RestartPlayer → SpawnDefaultPawn → FinishRestartPlayer → PossessedBy` 的中间，所以写在这里最稳。

## 4. 编辑器里还需要手工做的一次性配置

1. **Project Settings → Game → Aura Game Settings**
   - `Character Class Info` 已指向 `/Game/Blueprint/AbilitySystem/Data/DA_CharacterClassInfo`（`DefaultGame.ini` 里已预填）。
   - `Character Roster` 指向下一步建的 `DA_CharacterRoster`。
2. 新建 `DA_CharacterRoster`（父类 `UAuraCharacterRoster`），`Characters` 里加条目。
3. 每个可玩角色建一个 `DA_PlayerCharacter_*`（父类 `UPlayerCharacterDefinition`）：
   - `Pawn Class` 指向角色 BP（例如 `Bp_AuraCharacter`）
   - `Character Class` 选 `Warrior / Ranger / Elementalist / Tank`
   - 填 `Display Name` / `Icon` / `Sort Order`
4. 选人关卡自己配一个 `AFrontEndGameMode`（继承 `AGameModeBase`，**不要**继承 `AAuraGameModeBase`，
   否则菜单关会跑战斗逻辑）。

## 5. 联机时怎么改（现在不用做）

`UGameInstanceSubsystem` 只有一份，多个玩家会互相覆盖。真做联机时：

1. `ECharacterClass` / 选中的 `UPlayerCharacterDefinition` 放到 `AAuraPlayerState`，加 `Replicated` + `OnRep_`。
2. 选人界面在客户端调 `Server RPC` 上报给 `PlayerController`，由服务端写 `PlayerState`。
3. `AAuraGameModeBase::GetDefaultPawnClassForController` 从 `PlayerState` 取，而不是从 Subsystem 取。
4. 切换关卡用 seamless travel 时要 `CopyProperties`（`PlayerState` 会自动复制，Subsystem 不会）。

## 6. 常见坑

- **`TMap` 顺序不稳定**：职业表是 `TMap`，不要直接拿它当选人列表；用 `UAuraCharacterRoster` + `SortOrder`。
- **`GetSelectedCharacter()` 不主动加载**：它只返回「已加载」的资产，UI 里要显示数据请用 `TSoftObjectPtr::LoadSynchronous`
  或 `UAuraCharacterRoster::GetAllCharacters()`（后者会同步加载全部，数量很小，可以接受）。
- **菜单关 GameMode 继承错对象**：继承 `AAuraGameModeBase` 会顺带把战斗 Pawn、属性初始化、AI 全带进菜单关。
- **不要改用例表键**：`FCharacterClassDefaultInfo` 的键仍是 `ECharacterClass`，一个职业多个角色靠
  `UPlayerCharacterDefinition` 表达（例如两个 `Warrior` 角色共用同一套 GE，差异写在角色 BP 的 `AdditionalInitialEffects`）。
