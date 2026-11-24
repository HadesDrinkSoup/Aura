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
	
	/** 判断目标是否友好 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics", meta = (WorldContext = "WorldContextObject"))
	static bool IsFriend(AActor* FirstActor, AActor* SecondActor);
	
	/**
	 * 按武器挥砍轨迹做近战命中检测，碰撞形状可选（射线 / 球体 / 胶囊体 / 盒体）。
	 * 采样方式：
	 *  1. 每帧在刀身上（WeaponEndSocket → WeaponBeginSocket 这条刀身线）按 SubSteps 均分，
	 *     得到 SubSteps+1 个采样点，含刀柄与刀尖；
	 *  2. 每个采样点把「上一帧的世界坐标 → 当前帧的世界坐标」连成一条"检测段"，
	 *     再按 Shape 在这条检测段上做一次检测，一组检测段拼起来正好是整把刀这一帧扫过的空间。 
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static void AttackHitDetection(
		AActor* Attacker,
		FName BeginSocket,
		FName EndSocket,
		TArray<AActor*>& OutHitActors,
		const TArray<AActor*>& AlreadyHitActors,
		EWeaponHitShape Shape = EWeaponHitShape::Capsule,
		float Radius = 10.f,
		float HalfHeight = 0.f,
		int32 SubSteps = 6,
		bool bDrawDebug = false,
		float DebugDrawLifeTime = 0.05f);
};
