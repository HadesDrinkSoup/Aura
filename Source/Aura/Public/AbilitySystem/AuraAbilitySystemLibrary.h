// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UAttributesMenuController;
class UEnemyHealthBarWidgetController;
class UCharacterClassInfo;

/** 近战武器命中检测可选的碰撞形状 */
UENUM(BlueprintType)
enum class EWeaponHitShape : uint8
{
	/** 沿「上一帧位置 → 当前帧位置」这条检测段的直线，无粗细：最精确、最便宜；该检测段没有长度（该点两帧没动）时跳过 */
	Ray UMETA(DisplayName = "射线 (Ray)"),
	/** 以检测段中点为球心的球，Radius <= 0 时自动取段半长，把整段检测段包进一个球 */
	Sphere UMETA(DisplayName = "球体 (Sphere)"),
	/** 胶囊平躺在刀身上，长轴沿刀身（检测段方向绕刀面法线转 90°），中心是采样点；Radius 为半径，HalfHeight 在段半长之外再延伸 */
	Capsule UMETA(DisplayName = "胶囊体 (Capsule)"),
	/** 沿检测段的盒体：横截面半尺寸为 Radius，HalfHeight 在段半长之外再延伸 */
	Box UMETA(DisplayName = "盒体 (Box)")
};

/**
 * GAS 相关的全局查询库（C++ 与蓝图共用）
 *
 * 定位：只做「取对象 / 查配置 / 范围查询」，自身无状态、不创建 GE 与技能，因此所有函数都是 static，
 * 除范围查询外均声明为 BlueprintPure —— 在蓝图里是纯节点（没有执行引脚，可在任意表达式位置求值）；
 * GetLiveCharactersWithinRadius 要写输出数组且涉及物理查询，声明为 BlueprintCallable。
 *
 * 覆盖三类查询：
 *  1. UI 层：取本地 0 号玩家的 Overlay / 属性菜单 WidgetController（Widget 的数据源）；
 *  2. 数据层：从 GameMode 上取职业配置表（仅服务器有效）；
 *  3. 战斗查询层：球形重叠查询半径内存活角色（只读，不应用任何 GE 与技能）。
 *
 * 注意：
 *  - 全部是读操作，不要求服务器执行；但 UI 相关函数的返回值只在「有本地玩家」的端存在
 *    （客户端 / 监听服务器 / PIE 的客户端窗口），专用服务器上取不到，
 *    不要把 WidgetController 带进服务器侧的战斗逻辑；
 *  - 查找失败统一返回 nullptr（不崩溃、不抛异常），唯一例外是 PlayerState 为空这一种情况
 *    没有判空，见各函数说明；调用方一律要判空。
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** 获取叠加层WidgetController */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|Widget", meta = (WorldContext = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/** 获取属性菜单WidgetController */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|Character", meta = (WorldContext = "WorldContextObject"))
	static UAttributesMenuController* GetAttributesMenuController(const UObject* WorldContextObject);

	/** 获取敌人血条WidgetController */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|Widget", meta = (WorldContext = "WorldContextObject"))
	static UEnemyHealthBarWidgetController* GetElementHealthBarController(const UObject* WorldContextObject);
	
	/** 取全局职业配置表（AAuraGameModeBase上配置的DA_CharacterClassInfo） */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|Character", meta = (WorldContext = "WorldContextObject"))
	static const UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
	
	/** 获取是否闪避(应用效果上下文) */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsDodge (const FGameplayEffectContextHandle EffectContextHandle);
	
	/** 获取是否暴击(应用效果上下文) */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit (const FGameplayEffectContextHandle EffectContextHandle);
	
	/** 设置是否闪避(应用效果上下文) */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsDodge(FGameplayEffectContextHandle EffectContextHandle, bool bIsDodge);
	
	/** 设置是否暴击(应用效果上下文) */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(FGameplayEffectContextHandle EffectContextHandle, bool bIsCritical);

	/** 获取本次效果是什么元素的标签(应用效果上下文) */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);

	/** 设置本次效果是什么元素的标签(应用效果上下文) */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetElementType(FGameplayEffectContextHandle EffectContextHandle, const FGameplayTag& InDamageType);
	
	/** 查询球体范围内存活、实现了 ICombatInterface、且带指定Tag的角色，返回其 AvatarActor */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics", meta = (WorldContext = "WorldContextObject"))
	static void GetLiveCharactersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const AActor* IgnoreActor, FName RequiredTag, float Radius, const FVector& SpherePosition);
	
	/**
	 * 按武器挥砍轨迹做近战命中检测，碰撞形状可选（射线 / 球体 / 胶囊体 / 盒体）。
	 *
	 * 采样方式：
	 *  1. 每帧在刀身上（WeaponEndSocket → WeaponBeginSocket 这条刀身线）按 SubSteps 均分，
	 *     得到 SubSteps+1 个采样点，含刀柄与刀尖；
	 *  2. 每个采样点把「上一帧的世界坐标 → 当前帧的世界坐标」连成一条"检测段"，
	 *     再按 Shape 在这条检测段上做一次检测。
	 *  一组检测段拼起来正好是整把刀这一帧扫过的空间。
	 *
	 * 注意：两帧之间只连一条直线段（弦），不再沿弧线细分。刀转得很快或采样间隔很大时，
	 * 弦与真实弧线之间的空隙可能漏判、弦内侧也可能误判；这种情况要靠缩短调用间隔
	 * （定时器 0.01~0.05s）或加大 SubSteps 来补。
	 *
	 * 上一帧姿态由函数自己维护：内部按「攻击者 + 插槽名」缓存上一帧调用时的刀身姿态，
	 * 调用方每帧只传当前状态，不需要自己保存（也不该保存）上一帧位置。
	 * 只有第一次调用（还没有历史）时退化为只检测当前姿态。
	 *
	 * 挥拳/爪击等点攻击也支持：尖端与末端位置相同（或武器网格上只配了尖端一个插槽）时，
	 * 刀身退化成一个点、只采样刀尖一个点，检测段就是「上一帧拳头 → 当前帧拳头」的移动线。
	 *
	 * 形状语义（"检测段" = 某个采样点在两帧之间扫过的连线段）：
	 *  - Ray     ：沿检测段的直线，无粗细，最精确也最便宜；检测段没有长度（该点两帧没动）时跳过；
	 *  - Sphere  ：以检测段中点为球心的球，Radius<=0 时自动取段半长（把整段包进一个球）；
	 *  - Capsule ：胶囊平躺在刀身上，长轴沿刀身（等价于把检测段方向绕刀面法线旋转 90°），
	 *              中心仍是该采样点；Radius 为半径，HalfHeight 在段半长之外再延伸；
	 *              单插槽（没有根部插槽）时用武器网格包围盒的最长轴近似刀身方向；
	 *  - Box     ：沿检测段的盒体，横截面半尺寸为 Radius，HalfHeight 在段半长之外再延伸。
	 *
	 * 只负责"判定谁被命中"，不施加伤害/GE/技能：结算由调用方（通常只在服务器执行）拿着
	 * OutHitActors 自己做。返回的 Actor 已按 ICombatInterface::GetAvatarActor 归一化并去重。
	 *
	 * 已命中的目标不会重复检测：AlreadyHitActors 里的目标会被加进碰撞查询的忽略列表，
	 * 连物理检测都不做；同一次调用中前面采样点刚命中的目标也会立刻进入忽略列表，
	 * 因此一个目标一次攻击只被检测/返回一次。换目标照样能命中。
	 *
	 * @param Attacker            攻击者。函数会自动从它的骨骼网格组件里找武器网格：优先同时有两个插槽的，其次只有尖端插槽的
	 * @param WeaponBeginSocket   攻击点插槽名：武器是刀尖，挥拳时就是手掌/拳头插槽
	 * @param WeaponEndSocket     武器根部（刀柄）插槽名；可选。网格上没有该插槽、或与尖端相同时按点攻击处理
	 * @param OutHitActors        输出：本次调用新命中的存活角色（AvatarActor），已去重，不含 AlreadyHitActors 里的目标；函数内部会先 Reset
	 * @param AlreadyHitActors    输入：本次攻击已经命中过的角色列表（只读，函数不会修改它）。攻击开始时 Clear 一次，
	 *                            之后每帧把 OutHitActors Append 进这个数组、再把这个数组接回本引脚。
	 *                            列表里的目标会被加进碰撞查询的忽略列表——命中并加入后，后续帧与后续采样点都不再对它做
	 *                            物理检测，同一个敌人一次攻击只会结算一次伤害，换目标照样能命中
	 * @param Shape               碰撞形状，默认胶囊（和刀身贴合度最好；挥拳可用球体或胶囊）
	 * @param Radius              形状半径/横截面半尺寸（cm）：球体 <=0 表示自动包住整段检测段，其余形状 <=0 按 1cm；默认 10cm
	 * @param HalfHeight          胶囊/盒体在段半长之外额外增加的半长（cm），射线/球体忽略；默认 0
	 * @param SubSteps            刀身均分的段数：实际采样点数 = SubSteps+1（含刀柄与刀尖）。越大越细、越不容易漏判，开销也越高；默认 6
	 * @param bDrawDebug          是否绘制检测形状：按所选 Shape 绘制（射线画线，球/胶囊/盒画体积），未命中为红色、命中为绿色
	 * @param DebugDrawLifeTime   调试线存活时间（秒），默认 0.05；函数每帧调用，设太大会叠满屏幕。<=0 表示只画当前一帧
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static void WeaponAttackHitDetection(
		AActor* Attacker,
		FName WeaponBeginSocket,
		FName WeaponEndSocket,
		TArray<AActor*>& OutHitActors,
		const TArray<AActor*>& AlreadyHitActors,
		EWeaponHitShape Shape = EWeaponHitShape::Capsule,
		float Radius = 10.f,
		float HalfHeight = 0.f,
		int32 SubSteps = 6,
		bool bDrawDebug = false,
		float DebugDrawLifeTime = 0.05f);
};
