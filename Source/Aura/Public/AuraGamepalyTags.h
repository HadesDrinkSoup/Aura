// Study Game Ability System _Hades

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 单例全局访问
 * Aura GameplayTags 集中管理结构体
 * 用于统一注册和访问项目所有 GameplayTag，避免硬编码字符串
 *
 * 使用方式（全项目唯一的正确姿势）：
 *   1. 任意时刻只读访问：`const FAuraGameplayTags Tags = FAuraGameplayTags::Get();`（拿副本，避免外部改到单例）；
 *   2. 注册只发生一次：由 UAuraAssetManager::StartInitialLoading 调用 InitializeNativeGameplayTags()，
 *      而非某个 Actor 的构造/BeginPlay——AssetManager 早于所有游戏对象，能保证 tag 在使用前已注册；
 *   3. 不要用 RequestGameplayTag(FName("...")) 动态查，tag 没注册时会 assert；需要按名字反查请用 Get() 里的成员。
 *
 * 命名空间层级（tag 名 "A.B.C" 的点分结构，父级可被整体匹配，例如 Attributes.Derived 能匹配全部派生属性）：
 *   Attributes.Pool.*      —— 会被 GE 直接 Add/Set 的"当前值"属性
 *   Attributes.Primary.*   —— 玩家的基础属性点分配结果
 *   Attributes.Derived.*   —— 由 MMC 计算、再通过 GE 回写给角色的派生属性
 *   InputTag.*             —— 输入到技能的映射键，不是给 GE 用的
 *   Event.Montage.*        —— 动画通知（AnimNotify）在动画关键帧上发出的 GameplayEvent 名
 *
 * 注意：本结构体刻意不是 USTRUCT、不标 UPROPERTY——tag 是纯 C++ 单例数据，
 * 不参与 GC/反射/序列化；需要暴露给蓝图时请用单独的 UPROPERTY 变量拷贝出去。
 */
struct FAuraGameplayTags
{
	/**
	 * 取全局唯一实例（实际的实例是本结构体的静态成员 GameplayTags，进程启动即零初始化，
	 * 全部字段在 AssetManager 的 StartInitialLoading 阶段被填上真实 tag）。
	 * 返回 const 引用：调用方应复制一份到局部变量再用，不要持有引用跨帧（单例生命周期虽长，但避免误写）。
	 * 调用点：UAuraAttributeSet 构造函数用它构建「tag → 属性 Getter」映射表。
	 */
	static const FAuraGameplayTags& Get() {return GameplayTags;}

	/**
	 * 注册全部 native GameplayTag（名为 Initialize 但只做 AddNativeGameplayTag，可安全重复调用）。
	 * 调用点仅 UAuraAssetManager::StartInitialLoading() 一处，必须早于任何 GE/Ability 的加载与执行；
	 * 内部对每个 tag 都带了编辑器里可见的描述字符串，方便在 GameplayTag 编辑器中辨认。
	 */
	static void InitializeNativeGameplayTags();

	/** Pool Attributes — 运行时动态变动的当前值，直接受 GameplayEffect 修改 */
	FGameplayTag Attributes_Pool_Health;   // 当前生命值
	FGameplayTag Attributes_Pool_Mana;     // 当前法力值

	/** Primary Attributes — 基础属性，由装备/等级等系统直接修改，被 MMC 捕获作为派生属性计算的输入 */
	FGameplayTag Attributes_Primary_Strength;     // 力量 — 影响物理攻击力、最大生命值、护甲
	FGameplayTag Attributes_Primary_Intelligence; // 智力 — 影响魔法攻击力、最大法力值、魔法抗性、暴击率、冷却缩减、法力恢复
	FGameplayTag Attributes_Primary_Agility;      // 敏捷 — 影响物理攻击力、暴击率、命中率、闪避率、攻击速度、移动速度
	FGameplayTag Attributes_Primary_Vigor;        // 活力 — 影响最大生命值、最大法力值、护甲、魔法抗性、生命恢复

	/** Derived Attributes — 派生属性，由 MMC 根据 Primary 属性计算得出，通过 GameplayEffect 写入 */
	FGameplayTag Attributes_Derived_MaxHealth;         // 最大生命值 — 受 Vigor + Strength + 等级 影响，决定角色生命上限
	FGameplayTag Attributes_Derived_MaxMana;           // 最大法力值 — 受 Vigor + Intelligence + 等级 影响，决定角色法力上限
	FGameplayTag Attributes_Derived_PhysicalAttack;    // 物理攻击力 — 受 Strength + Agility + 等级 影响，决定物理伤害基础数值
	FGameplayTag Attributes_Derived_MagicalAttack;     // 魔法攻击力 — 受 Intelligence + 等级 影响，决定魔法伤害基础数值
	FGameplayTag Attributes_Derived_Armor;             // 护甲 — 受 Vigor + Strength + 等级 影响，降低受到的物理伤害
	FGameplayTag Attributes_Derived_MagicResistance;   // 魔法抗性 — 受 Vigor + Intelligence + 等级 影响，降低受到的魔法伤害
	FGameplayTag Attributes_Derived_CriticalChance;    // 暴击几率 — 受 Agility + Intelligence 影响，决定攻击暴击的概率
	FGameplayTag Attributes_Derived_HitChance;         // 命中几率 — 受 Agility + 等级 影响，决定攻击命中目标的概率
	FGameplayTag Attributes_Derived_DodgeChance;       // 闪避几率 — 受 Agility 影响，决定角色闪避攻击的概率
	FGameplayTag Attributes_Derived_AttackSpeed;       // 攻击速度 — 受 Agility + 等级 影响，决定角色攻击频率
	FGameplayTag Attributes_Derived_CooldownReduction; // 冷却缩减 — 受 Intelligence 影响，降低技能冷却时间
	FGameplayTag Attributes_Derived_MoveSpeed;         // 移动速度 — 受 Agility 影响，决定角色移动速度
	FGameplayTag Attributes_Derived_HealthRegen;       // 生命恢复 — 受 Vigor + 等级 影响，决定每秒自动恢复的生命值
	FGameplayTag Attributes_Derived_ManaRegen;         // 法力恢复 — 受 Intelligence + 等级 影响，决定每秒自动恢复的法力值
	
	/** 攻击效果 */
	FGameplayTag Damage;						//攻击伤害
	FGameplayTag Effects_HitReact;				//攻击命中反馈
	/**
	 * InputTag 组：技能蓝图的输入标识，不是属性、也不参与 GE。
	 *
	 * 数据流：DA_InputConfig 里把输入 Action 映射到某个 InputTag →
	 *        UAuraEnhancedInputComponent::BindAbilityActions 把该 tag 作为参数绑到
	 *        PlayerController / AbilitySystemComponent 的 AbilityInputTagPressed/Held/Released →
	 *        ASC 用 AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) 找到要激活的技能
	 *        （tag 是角色 AddCharacterAbilities 时从 UAuraGameplayAbility::AbilityInputTag 写进技能实例的）。
	 *
	 * 因此判断"哪个键触发哪个技能"只依赖 tag 名相等（HasTagExact 精确匹配，父级 tag 不会误命中），
	 * 这里的命名和 GA 蓝图里配置的 InputTag 必须完全一致；C++ 代码中不引用这些成员。
	 */
	FGameplayTag InputTag_LMB; // 鼠标左键 — 普攻/主技能，GA_FireBall 与 GA_TestAbility 都用它
	FGameplayTag InputTag_RMB; // 鼠标右键 — 次要技能槽（当前无技能资产占用）
	FGameplayTag InputTag_1;   // 数字键 1 — 技能槽位 1（当前无技能资产占用）
	FGameplayTag InputTag_2;   // 数字键 2 — 技能槽位 2（当前无技能资产占用）
	FGameplayTag InputTag_3;   // 数字键 3 — 技能槽位 3（当前无技能资产占用）
	FGameplayTag InputTag_4;   // 数字键 4 — 技能槽位 4（当前无技能资产占用）

	/**
	 * 动画事件 tag：火球施法动画（AM_Cast_FireBolt）在关键帧上用 AnimNotify 发出 GameplayEvent，
	 * 技能蓝图通过 WaitGameplayEvent 节点按这个 tag 接收，用于在"火球脱手"的那一帧生成投射物。
	 * 属于 Event.Montage.* 命名空间，由动画资产引用，C++ 中没有直接引用。
	 * 注意它和 InputTag_* 是两条独立链路：Input 负责"按键触发技能"，Event 负责"动画播到某帧时通知技能"。
	 */
	FGameplayTag Event_Montage_FireBall;

private:
	/** 全局唯一实例：进程启动时零初始化（tag 均为空），由 InitializeNativeGameplayTags 填入真实值 */
	static FAuraGameplayTags GameplayTags;
};
