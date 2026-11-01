// Study Game Ability System _Hades
//
// AuraGameplayTags.cpp — Native GameplayTag 集中注册
//
// 架构概述：
//   Pool 属性    — 运行时实际生效的值（当前血量/蓝量），被 GE 实时修改
//   Primary 属性 — 角色成长的投入维度（力量/智力/敏捷/活力），玩家加点/装备提供
//   Derived 属性 — 由 Primary + 等级 经 MMC 计算得出，作为战斗公式的实际参数
//
//   AddNativeGameplayTag 的第二个参数是编辑器内对设计师显示的描述文本。

/**
 * @file AuraGamepalyTags.cpp
 * @brief 全项目 native GameplayTag 的集中注册实现（AuraGameplayTags.h 里那个单例的填充处）。
 *
 * 要点：
 *  - 全部 tag 都在这里一次性 Add 完，**不写进 DefaultGameplayTags.ini**：native 注册的好处是
 *    改了 tag 名一定跟着代码走，不会出现"ini 里删了、代码里还在用"的悬空引用；
 *  - 每个 Add 都带描述字符串，GameplayTag 编辑器/资产里的 tooltip 就是它，改语义时记得同步改描述；
 *  - 注册时机由 UAuraAssetManager::StartInitialLoading 保证（早于任何 GE/Ability/GAS 初始化），
 *    因此本函数内部不需要判重、不需要加锁，也不应被其他代码调用第二次（重复调用虽安全但没意义）；
 *  - 这里的 tag 只被 UAuraAttributeSet 的构造（属性 tag→Getter 映射）和资产（输入配置、GA、动画通知）消费，
 *    C++ 战斗逻辑里不要按字符串硬编码，一律走 FAuraGameplayTags::Get()。
 */

#include "AuraGamepalyTags.h"
#include "GameplayTagsManager.h"

/**
 * 单例的存储本体（静态成员，写在 .cpp 里避免头文件里出现多处定义）。
 * 进程启动时零初始化，所有 FGameplayTag 为空；只有 InitializeNativeGameplayTags 会写入真实值。
 */
FAuraGameplayTags FAuraGameplayTags::GameplayTags;

/**
 * 把下面所有属性逐个注册到 GameplayTagManager，并回填到单例。
 * 调用时机：UAuraAssetManager::StartInitialLoading（引擎完成资源管理器初始化后、任何游戏对象存在之前）。
 * 无返回值、无失败分支：AddNativeGameplayTag 对同名 tag 是幂等的，失败也不会返回空（除非名称为非法 FName）。
 */
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	// =========================================================================
	// Pool Attributes — 运行时池值
	// =========================================================================

	// 当前生命值：角色是否存活的唯一判定。受伤害减少、受治疗增加。<=0 则死亡
	GameplayTags.Attributes_Pool_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Pool.Health"),
		FString(TEXT("当前生命值。角色存活判定：>0 存活，<=0 死亡。受伤害扣减，受治疗恢复。上限受 MaxHealth 限制")));

	// 当前法力值：施放技能的消耗资源。不足时技能图标灰化、无法施放
	GameplayTags.Attributes_Pool_Mana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Pool.Mana"),
		FString(TEXT("当前法力值。施放技能的资源消耗池，不足时无法施放技能。上限受 MaxMana 限制")));

	// =========================================================================
	// Primary Attributes — 基础属性（玩家投入加点的四个维度）
	// =========================================================================

	// 力量：物理职业主属性。每点力量 → 物理攻击力↑、最大生命值↑、护甲↑。适合战士/坦克投入
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Strength"),
		FString(TEXT("力量，提升：物理攻击力、最大生命值、护甲")));

	// 智力：魔法职业主属性。每点智力 → 魔法攻击力↑、最大法力值↑、魔法抗性↑、暴击率↑、冷却缩减↑、法力恢复↑。适合法师投入
	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence"),
		FString(TEXT("智力，提升：魔法攻击力、最大法力值、魔法抗性、暴击率、冷却缩减、法力恢复")));

	// 敏捷：灵巧职业主属性。每点敏捷 → 物理攻击力↑、暴击率↑、命中率↑、闪避率↑、攻击速度↑、移动速度↑。适合游侠/刺客投入
	GameplayTags.Attributes_Primary_Agility = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Agility"),
		FString(TEXT("敏捷，提升：物理攻击力、暴击率、命中率、闪避率、攻击速度、移动速度")));

	// 活力：通用生存属性。每点活力 → 最大生命值↑、最大法力值↑、护甲↑、魔法抗性↑、生命恢复↑。所有职业都需要适量投入
	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor"),
		FString(TEXT("活力，提升：最大生命值、最大法力值、护甲、魔法抗性、生命恢复")));

	// =========================================================================
	// Derived Attributes — 派生属性（战斗公式的实际参数）
	// =========================================================================

	// 最大生命值：决定角色能承受多少总伤害。生命条的长度。坦克能力的核心
	// 来源：Vigor(主) + Strength + 等级
	GameplayTags.Attributes_Derived_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.MaxHealth"),
		FString(TEXT("最大生命值，决定生命条上限，限制 Pool.Health 的最大值。来源：Vigor + Strength + 等级")));

	// 最大法力值：决定角色能连续施放多少个技能。法力条的长度。法师续航的根基
	// 来源：Vigor + Intelligence(主) + 等级
	GameplayTags.Attributes_Derived_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.MaxMana"),
		FString(TEXT("最大法力值，决定法力条上限，限制 Pool.Mana 的最大值。来源：Vigor + Intelligence + 等级")));

	// 物理攻击力：所有物理伤害技能的伤害计算基数。技能伤害 = 物攻 × 技能倍率 - 敌方护甲
	// 来源：Strength(主) + Agility + 等级
	GameplayTags.Attributes_Derived_PhysicalAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.PhysicalAttack"),
		FString(TEXT("物理攻击力，物理技能与普攻的伤害计算基数。技能最终伤害 = 物攻 × 倍率 - 护甲减免。来源：Strength + Agility + 等级")));

	// 魔法攻击力：所有魔法伤害技能的伤害计算基数。技能伤害 = 魔攻 × 技能倍率 - 敌方魔抗
	// 来源：Intelligence(主) + 等级
	GameplayTags.Attributes_Derived_MagicalAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.MagicalAttack"),
		FString(TEXT("魔法攻击力，魔法技能的伤害计算基数。技能最终伤害 = 魔攻 × 倍率 - 魔抗减免。来源：Intelligence + 等级")));

	// 护甲：减免受到的物理伤害。伤害减免% = 护甲 / (护甲 + 常数)。坦克前排站得住的关键
	// 来源：Vigor(主) + Strength + 等级
	GameplayTags.Attributes_Derived_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.Armor"),
		FString(TEXT("护甲，按公式转换为物理伤害减免百分比。来源：Vigor + Strength + 等级")));

	// 魔法抗性：减免受到的魔法伤害。伤害减免% = 魔抗 / (魔抗 + 常数)。对抗法师 Boss 的核心防御
	// 来源：Vigor(主) + Intelligence + 等级
	GameplayTags.Attributes_Derived_MagicResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.MagicResistance"),
		FString(TEXT("魔抗，按公式转换为魔法伤害减免百分比。来源：Vigor + Intelligence + 等级")));

	// 暴击几率：攻击造成暴击（倍率伤害，通常 150%~200%）的概率。提升输出的关键手段
	// 来源：Agility(主) + Intelligence
	GameplayTags.Attributes_Derived_CriticalChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.CriticalChance"),
		FString(TEXT("暴击率，攻击触发暴击伤害（默认1.5~2.0倍）的概率百分比。来源：Agility + Intelligence")));

	// 命中几率：攻击命中目标的概率。敌方闪避率会降低实际命中。确保伤害不落空
	// 来源：Agility(主) + 等级
	GameplayTags.Attributes_Derived_HitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.HitChance"),
		FString(TEXT("命中率，决定攻击能否命中目标。实际命中 = 命中率 - 敌方闪避率。来源：Agility + 等级")));
	
	// 闪避几率：完全躲避一次攻击的概率。触发时该次伤害归零。敏捷角色的核心生存机制
	// 来源：Agility
	GameplayTags.Attributes_Derived_DodgeChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.DodgeChance"),
		FString(TEXT("闪避率，完全躲避攻击的概率，触发时伤害为0。来源：Agility")));

	// 攻击速度：普通攻击的攻击频率加成。每点提升普攻的每秒攻击次数，直接拉高 DPS
	// 来源：Agility(主) + 等级
	GameplayTags.Attributes_Derived_AttackSpeed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.AttackSpeed"),
		FString(TEXT("攻击速度，普攻频率加成，直接提升每秒普攻次数和DPS。来源：Agility + 等级")));

	// 冷却缩减：减少技能冷却时间的百分比。技能冷却 = 原始CD × (1 - 冷却缩减%)。技能流玩法的质变属性
	// 来源：Intelligence
	GameplayTags.Attributes_Derived_CooldownReduction = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.CooldownReduction"),
		FString(TEXT("冷却缩减，按百分比减少技能冷却时间，提升技能循环频率。技能流质变属性。来源：Intelligence")));

	// 生命恢复：每秒自动恢复的生命值。减少对治疗和血瓶的依赖，提供脱战续航
	// 来源：Vigor(主) + 等级
	GameplayTags.Attributes_Derived_HealthRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.HealthRegen"),
		FString(TEXT("生命恢复，每秒自动恢复的生命值。提供脱战续航，减少对治疗的依赖。来源：Vigor + 等级")));

	// 法力恢复：每秒自动恢复的法力值。减少对蓝瓶的依赖，让法师在持久战中不会空蓝
	// 来源：Intelligence(主) + 等级
	GameplayTags.Attributes_Derived_ManaRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.ManaRegen"),
		FString(TEXT("法力恢复，每秒自动恢复的法力值。在持久战中维持法力循环。来源：Intelligence + 等级")));

	// 移动速度：角色在地图上的移动快慢。决定走位、风筝、追击、逃跑的效率。通用战术属性
	// 来源：Agility
	GameplayTags.Attributes_Derived_MoveSpeed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Derived.MoveSpeed"),
		FString(TEXT("移动速度，角色移动速率。影响走位/风筝/追击/逃跑效率。来源：Agility")));
	
	// =========================================================================
	// 攻击效果
	// =========================================================================
	
	// 攻击伤害
	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage"),
		FString(TEXT("伤害")));
	// 攻击命中反馈
	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.HitReact"),
		FString(TEXT("命中反馈")));
	
	// =========================================================================
	// Input Tags — 输入到技能的映射键（不参与 GE，纯"哪个键对应哪个技能"的标识）
	//
	// 匹配方式：ASC 的 AbilityInputTagPressed/Held/Released 里用 HasTagExact 精确匹配技能的
	// AbilityInputTag（来自 GA 蓝图配置），所以这里每个 tag 都是独立的叶子节点，父子匹配在这里不生效。
	// 消费方：Content/Blueprint/Input/DA_InputConfig（键位 → tag）与各 GA 蓝图（tag → 技能）。
	// C++ 代码对这些成员没有直接引用。
	// =========================================================================

	// 左键：主技能键。GA_FireBall、GA_TestAbility 都用它触发，是当前唯一真正在用的输入槽
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"), 
		FString(TEXT("鼠标左键输入标签")));
	
	// 右键：次要技能槽。DA_InputConfig 已映射键位，但当前没有 GA 资产占用该 tag（预留）
	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"), 
		FString(TEXT("鼠标右键键输入标签")));
	
	// 数字键 1：技能槽 1。键位已映射，当前无 GA 占用（预留）
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"), 
		FString(TEXT("数字1键输入标签")));
	
	// 数字键 2：技能槽 2。键位已映射，当前无 GA 占用（预留）
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"), 
		FString(TEXT("数字2键输入标签")));
	
	// 数字键 3：技能槽 3。键位已映射，当前无 GA 占用（预留）
	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"), 
		FString(TEXT("数字3键输入标签")));
	
	// 数字键 4：技能槽 4。键位已映射，当前无 GA 占用（预留）
	GameplayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"), 
		FString(TEXT("数字4键输入标签")));
	
	GameplayTags.Event_Montage_FireBall = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.Montage.FireBall"), 
		FString(
			TEXT("用于火球术动画通知")));
}
