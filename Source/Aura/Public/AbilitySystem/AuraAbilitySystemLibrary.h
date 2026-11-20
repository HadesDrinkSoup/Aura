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
	/** 沿检测段的直线，无粗细：最精确、最便宜；检测段没有长度（该点两帧没动）时跳过 */
	Ray UMETA(DisplayName = "射线 (Ray)"),
	/** 以检测段中点为球心的球：半径 <= 0 时自动取段半长，把整段包成一个球 */
	Sphere UMETA(DisplayName = "球体 (Sphere)"),
	/** 沿检测段的胶囊体：Radius 为半径，HalfHeight 在段半长之外再延伸 */
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
	static void SetDamageType(FGameplayEffectContextHandle EffectContextHandle, const FGameplayTag& InDamageType);
	
	/** 查询球体范围内存活、实现了 ICombatInterface、且带指定Tag的角色，返回其 AvatarActor */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics", meta = (WorldContext = "WorldContextObject"))
	static void GetLiveCharactersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const AActor* IgnoreActor, FName RequiredTag, float Radius, const FVector& SpherePosition);
	
	/**
	 * 按武器挥砍轨迹做近战命中检测，碰撞形状可选（射线 / 球体 / 胶囊体 / 盒体）。
	 *
	 * 原理：WeaponEndSocket → WeaponBeginSocket 之间是一条"刀身线段"。攻击期间每帧调用本函数，
	 * 传入上一帧与当前帧的两个端点；函数在刀身上按 SubSteps 均分出 SubSteps+1 个采样点
	 * （含刀柄与刀尖），每个采样点把「上一帧位置 → 当前帧位置」连成一条"检测段"，
	 * 再按 Shape 在每条检测段上做一次检测。于是命中范围是整把刀这一帧扫过的空间，
	 * 弧线挥砍、刀身平移都自然包含在这些检测段里，函数内部不再插值刀身姿态。
	 *
	 * 挥拳/爪击等点攻击也支持：尖端与末端位置相同（或武器网格上只配了尖端一个插槽）时，
	 * 刀身退化成一个点、采样点只剩一个，那条检测段就是「上一帧拳头 → 当前帧拳头」的移动连线。
	 *
	 * 形状语义（"检测段" = 某个采样点在上一帧位置与当前帧位置之间的连线）：
	 *  - Ray     ：沿检测段的直线，无粗细，最精确也最便宜；检测段没有长度（该点两帧没动）时跳过；
	 *  - Sphere  ：以检测段中点为球心的球，Radius<=0 时自动取段半长（把整段包进一个球）；
	 *  - Capsule ：沿检测段的胶囊，Radius 为半径，HalfHeight 在段半长之外再延伸；
	 *  - Box     ：沿检测段的盒体，横截面半尺寸为 Radius，HalfHeight 在段半长之外再延伸。
	 *
	 * 只负责"判定谁被命中"，不施加伤害/GE/技能：结算由调用方（通常只在服务器执行）拿着
	 * OutHitActors 自己做。返回的 Actor 已按 ICombatInterface::GetAvatarActor 归一化并去重。
	 *
	 * @param Attacker            攻击者。函数会自动从它的骨骼网格组件里找武器网格：优先同时有两个插槽的，其次只有尖端插槽的
	 * @param WeaponBeginSocket     攻击点插槽名：武器是刀尖，挥拳时就是手掌/拳头插槽
	 * @param WeaponEndSocket     武器根部（刀柄）插槽名；可选。网格上没有该插槽、或与尖端相同时按点攻击处理
	 * @param PreviousBeginLocation 上一帧的尖端世界坐标。攻击开始时把它初始化成当前尖端坐标即可；
	 *                            传全零会被当作"还没有上一帧"，退化为只检测当前姿态
	 * @param PreviousEndLocation 上一帧的根部世界坐标，规则同上；点攻击时与上一帧尖端相同
	 * @param OutHitActors        输出：本次调用新命中的存活角色（AvatarActor），已去重；函数内部会先 Reset
	 * @param AlreadyHitActors    输入：本次攻击已经命中过的角色列表（只读）。攻击开始时 Clear 一次，
	 *                            之后每帧把 OutHitActors Append 进这个数组、再把这个数组接回本引脚。
	 *                            函数不会往数组里写，所以数组内容完全由蓝图控制；列表里的目标会被加进
	 *                            碰撞查询的忽略列表——第一次命中并加入后，后续帧与后续采样点都不再对
	 *                            它做物理检测（因此已命中的刀段调试显示为红色）。同一个敌人一次攻击
	 *                            只会结算一次伤害，换目标照样能命中
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
		const FVector& PreviousBeginLocation,
		const FVector& PreviousEndLocation,
		TArray<AActor*>& OutHitActors,
		const TArray<AActor*>& AlreadyHitActors,
		EWeaponHitShape Shape = EWeaponHitShape::Capsule,
		float Radius = 10.f,
		float HalfHeight = 0.f,
		int32 SubSteps = 2,
		bool bDrawDebug = false,
		float DebugDrawLifeTime = 0.05f);
};
