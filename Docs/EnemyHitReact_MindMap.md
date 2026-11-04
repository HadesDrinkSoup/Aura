# 敌人受击（HitReact）流程

_伤害 GE · 受击蒙太奇 · 标签停步 · AURA / UE5 GAS_

## ① 总览 — 一条链上的 7 次跳转

- 玩家输入 → `GA_FireBall` → `UAuraProjectileSpell::SpawnProjectile()`：技能端
- → `AAuraProjectile::OnSphereComponentBeginOverlap()`：命中判定
- → `ApplyGameplayEffectSpecToSelf()`：投射物把伤害 GE 拍给受击者
- → `UAuraAttributeSet::PostGameplayEffectExecute()`：伤害落到 Health
- → `TryActivateAbilitiesByTag(Effects.HitReact)`：扣血转「受击」
- → `GA_HitReact` → `PlayMontageAndWait` + `ApplyGameplayEffectToOwner(GE_HitReact)`：动画 + 挂标签
- → `AAuraEnemy::HitReactTagChanged()`：标签变化 → `MaxWalkSpeed = 0`

调用链一行版：`SpawnProjectile（客户端早退 / 服务器继续） → SpawnActorDeferred → OnBeginOverlap → PostGameplayEffectExecute → TryActivateAbilitiesByTag → GA_HitReact → HitReactTagChanged`

## ② 发射端 — 伤害 Spec 在哪造

- `UAuraProjectileSpell::SpawnProjectile()`：蓝图入口；客户端在这里直接 return，不发请求（AuraProjectileSpell.cpp:57，守卫在 :64）
- 客户端的目标点由 `UTargetDataUnderMouse::SendMouseCursorData()` 通过 `ServerSetReplicatedTargetData`（GAS 内置 Server RPC）上传（TargetDataUnderMouse.cpp:137）
- 服务器端同一份技能实例（GA_FireBall 是 Local Predicted）收到目标数据后，蓝图再次调用 `SpawnProjectile`，这次走服务器分支
- `MakeOutgoingSpec(DamageEffectClass, SetAbilityLevel(), MakeEffectContext())`：造 Spec（AuraProjectileSpell.cpp:108）
- `SetAbilityLevel()`：技能等级 = 施法者角色等级（AuraGameplayAbility.cpp:22-40）
- `Damage.GetValueAtLevel(AbilityLevel)`：FScalableFloat 按角色等级查 CurveTable（AuraProjectileSpell.cpp:110，字段见 AuraGameplayAbility.h:44）
- `AssignTagSetByCallerMagnitude(Spec, Damage, ScaledDamage)`：把数值塞进 Spec（:119）
- `SpawnActorDeferred` → 先赋 `DamageEffectSpecHandle` 再 `FinishSpawning`：保证 BeginPlay 绑 Overlap 前句柄就绪（:92-121）

## ③ 命中判定 — 投射物

- `AAuraProjectile::OnSphereComponentBeginOverlap()`：唯一伤害入口（AuraProjectile.cpp:126）
- `if (!HasAuthority()) return;`：只在服务端结算
- 过滤：`OtherActor == this` / `== GetOwner()` 直接跳过
- `SweepResult.ImpactPoint / ImpactNormal`：撞点法线，零值时退回自身位置与 `-Forward`
- `Multicast_OnHit(Location, Normal)`：先广播表现（:161）
- `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)`：目标无 ASC 则只播特效不结算
- `ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get())`：把伤害 GE 打在目标自己身上（:165-168）
- `Destroy()`：一发一次，命中即销毁（:172）
- `Multicast_OnHit_Implementation()`：NetMulticast Reliable，Niagara 命中特效 + `PlaySoundAtLocation` + 停循环音（:183）

## ④ 伤害结算 — Instant GE 落到 Health

- `GE_Damage`：Instant，Modifier 打 `AuraAttributeSet.IncomingDamage`，`Add(AddBase)` + SetByCaller(Damage)
- `UAuraAttributeSet::PostGameplayEffectExecute()`：属性最终裁定（AuraAttributeSet.cpp:249）
- `SetEffectProperties(Data, EffectProperties)`：沿 EffectContext 反查施法者/目标 ASC（:461）
- 四种钳制：Health / Mana / Stamina 等先 Clamp 到 `[0, Max]`
- `EvaluatedData.Attribute == GetIncomingDamageAttribute()`：分离出「待结算伤害」（:280）
- `LocalIncomingDamage = GetIncomingDamage()` → `SetIncomingDamage(0)`：Meta 属性用完即清
- `NewHealth = GetHealth() - LocalIncomingDamage` → `SetHealth(Clamp(NewHealth, 0, MaxHealth))`
- `bFatal = NewHealth <= 0.f`：致命标记（目前无人消费，见 ⑧）
- 非致命 → `AddTag(Effects_HitReact)` → `TargetASC->TryActivateAbilitiesByTag(TagContainer)`：**扣血转动画的唯一衔接点**（:293-295）
- `IncomingDamage` 无 Replicated（AuraAttributeSet.h:379-381）：客户端不需要它

## ⑤ 受击技能 — GA_HitReact

- 授予：`AAuraCharacterBase::GetCombinedStartupAbilities()`：BP + 职业 CommonAbilities（AuraCharacterBase.cpp:81）
- 敌人：`InitActorInfo()` → `AddCharacterAbilities()`（AuraEnemy.cpp:77）
- 激活：`TryActivateAbilitiesByTag` 按 `Effects.HitReact` 找 AbilityTag 匹配的 GA
- `ICombatInterface::GetHitReactMontage()`：BlueprintNativeEvent（CombatInterface.h:63）
- `AAuraCharacterBase::GetHitReactMontage_Implementation()`：返回 `HitReactMontage`（AuraCharacterBase.cpp:56）
- `CreatePlayMontageAndWaitProxy()`：`AbilityTask_PlayMontageAndWait`，PlayRate / StartSection 默认
- `BP_ApplyGameplayEffectToOwner(GE_HitReact)`：给「拥有者自己」挂 GE，返回 `ActiveGameplayEffectHandle`
- `OnCompleted / OnBlendOut / OnInterrupted / OnCancelled` → `K2_EndAbility`：四条出口都收敛到结束技能
- `GE_HitReact`：**Infinite**，`InheritableOwnedTagsContainer` 授予 `Effects.HitReact`

## ⑥ 标签 → 停步 — HitReactTagChanged

- `RegisterGameplayTagEvent(Effects_HitReact, NewOrRemoved).AddUObject(this, &AAuraEnemy::HitReactTagChanged)`：BeginPlay 注册（AuraEnemy.cpp:62-63）
- 用 `NewOrRemoved` 而非 `AnyCountChange`：只要 0↔1 的进出
- `bHitReact = NewCount > 0`：BlueprintReadOnly 供动画蓝图/后续逻辑读（AuraEnemy.h:56-57）
- `GetCharacterMovement()->MaxWalkSpeed = bHitReact ? 0.f : BaseWalkSpeed`：受击期原地站定（:151）
- `BaseWalkSpeed` 默认 250：恢复时回写这个值，不要写死 600
- 敌人 ASC 用 `EGameplayEffectReplicationMode::Minimal`（AuraEnemy.cpp:40）：标签只复制给模拟端

## ⑦ 表现层 — 双端各画什么

- 命中特效/音效：`Multicast_OnHit_Implementation`，客户端与服务端都会执行
- 受击动画：`AbilityTask_PlayMontageAndWait`，蒙太奇本身走正常 AnimMontage 复制
- 血条：`OnRep_Health` → `GAMEPLAYATTRIBUTE_REPNOTIFY`（AuraAttributeSet.cpp:315）→ `UEnemyHealthBarWidgetController`
- 血条创建：`AAuraEnemy::InitHealthBar()`（AuraEnemy.cpp:89）
- 伤害数字飘字：本项目**未实现**，需要自己加 `GameplayCue` 或读 `LocalIncomingDamage`

## ⑧ 坑与待办 — 改这里之前必读

- `bFatal` 算出来没人用：全项目无死亡处理，血量归零既不播死亡蒙太奇也不销毁
- `GA_HitReact` 未 `RemoveActiveGameplayEffect` + `GE_HitReact` 是 Infinite → `Effects.HitReact` 常驻，敌人**受击一次后永久停步**
- 两种修法：OnEndAbility 补 `RemoveActiveGameplayEffect(ActiveGameplayEffectHandle)`，或把 GE 改成 Duration（0.5s 左右）自动过期
- 投射物无 `bHit` 去重（AuraProjectile.cpp:120-124）：同帧命中多目标可能重复结算/重复 Multicast
- `DamageEffectSpecHandle` 直接解引用未判空（:167）：非技能途径生成的投射物会崩
- `DamageEffectSpecHandle` 未标 Replicated（AuraProjectile.h:80）：客户端命中路径拿不到有效句柄（当前靠 HasAuthority 规避）
- `PostGameplayEffectExecute` 两处 `UE_LOG` 直接解引用 `TargetAvatarActor` 未判空（AuraAttributeSet.cpp:263、:288）
- 受击动画与相机、其他 Montage 的优先级未处理：受击期间再次受击会 `OnInterrupted` 直接 EndAbility，但 GE 仍无限期叠加
