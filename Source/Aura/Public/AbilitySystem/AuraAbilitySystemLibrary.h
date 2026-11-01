// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UAttributesMenuController;
class UCharacterClassInfo;
class UAuraCharacterRoster;
/**
 * GAS 相关的全局查询库（C++ 与蓝图共用）
 *
 * 定位：只做「取对象 / 查配置」，自身无状态、不创建 GE 与技能，因此所有函数都是 static，
 * 且声明为 BlueprintPure —— 在蓝图里是纯节点（没有执行引脚，可在任意表达式位置求值）。
 *
 * 覆盖两类查询：
 *  1. UI 层：取本地 0 号玩家的 Overlay / 属性菜单 WidgetController（Widget 的数据源）；
 *  2. 数据层：取 Project Settings 里配置的职业配置表与可选角色名单。
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
	/**
	 * 取本地 0 号玩家的 Overlay WidgetController（主 HUD：血球、蓝球、技能图标的数据源）。
	 *
	 * 链路：UGameplayStatics::GetPlayerController(0) → AAuraPlayerController → AAuraHUD
	 *      → 用该玩家的 PlayerState 组装 FWidgetControllerParams(PC, PS, ASC, AS)
	 *      → AAuraHUD::GetOverlayWidgetController（HUD 内懒加载，同一 HUD 生命周期内只创建一次并缓存）。
	 *
	 * 用途：子 Widget / 蓝图工具需要绕过 HUD 逐层 Cast 直接拿到数据源时调用。
	 * 调用时机：必须在 HUD 完成 Overlay 初始化之后（此时 Controller 已创建并绑好回调），
	 *          在 HUD 初始化之前调用虽然会新建 Controller，但拿到的实例不会被 Widget 绑定。
	 *
	 * 失败返回 nullptr 的情形：取不到 0 号 PlayerController、PC 不是 AAuraPlayerController、
	 *   PC 的 HUD 不是 AAuraHUD。专用服务器上必然为 nullptr（没有本地玩家）。
	 * 注意：本函数没有对 GetPlayerState 的结果判空，PlayerState 尚未就绪就调用会空指针解引用；
	 *      同理 BP_AuraHUD 里的 OverlayWidgetControllerClass 未配置时，HUD 侧新建对象会失败。
	 *
	 * @param WorldContextObject 仅用于蓝图侧隐式注入世界上下文，函数内部固定取玩家索引 0，不参与查找。
	 */
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/**
	 * 取本地 0 号玩家的属性菜单（Attributes Menu）WidgetController。
	 *
	 * 与 GetOverlayWidgetController 完全同构，区别只在目标换成 AAuraHUD 缓存的
	 * AttributesMenuController；该 Controller 在 Overlay 初始化时已被提前创建并绑好回调
	 * （见 AAuraHUD::InitAttributesMenu），所以这里通常只是取缓存，不新建对象。
	 *
	 * 用途：属性加点界面（Widget 由蓝图按钮点击时再懒创建）读写属性值、执行加点。
	 * 失败返回 nullptr 的条件、以及 PlayerState 未判空的注意事项，与 GetOverlayWidgetController 相同。
	 *
	 * @param WorldContextObject 仅用于蓝图侧隐式注入世界上下文，玩家索引固定为 0。
	 */
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static UAttributesMenuController* GetAttributesMenuController(const UObject* WorldContextObject);

	/**
	 * 取全局职业配置表（UAuraGameSettings 上配置的 DA_CharacterClassInfo）。
	 * 配置是进程级静态数据，不按世界查找；WorldContextObject 只用于 BP 侧自动填上下文。
	 *
	 * 内部实现：UAuraGameSettings::Get()（CDO，只读）→ 对 TSoftObjectPtr 调 LoadSynchronous()，
	 * 即首次调用会同步加载该 DataAsset（阻塞主线程），所以应只在初始化期调用，不要放进 Tick。
	 *
	 * 调用方：AAuraCharacterBase::FindClassDefaultInfo（解析职业默认的 Primary/Derived/Pool GE 与默认技能）、
	 *        UPlayerCharacterDefinition（读职业的展示信息）。
	 *
	 * 失败返回：未配置或软引用加载失败时返回 nullptr，并在 AuraGameSettings 里打一条 Warning；
	 * 此时整条属性/技能下发链都会空转（角色不会获得任何属性），调用方必须判空。
	 * 无需服务器执行：纯静态配置读取，客户端读到的也是同一份资产。
	 *
	 * @param WorldContextObject 未参与查找，仅为蓝图节点的世界上下文占位。
	 */
	UFUNCTION(BlueprintPure, Category = "Aura|Character", meta = (WorldContext = "WorldContextObject"))
	static const UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	/**
	 * 取可选角色名单（UAuraGameSettings 上配置的 DA_CharacterRoster）；选人界面用。
	 *
	 * 同样走 CDO + LoadSynchronous，属于同步加载的静态配置。
	 * C++ 侧目前没有调用点（UAuraCharacterRoster 自己的接口先留好），预期由选人界面的蓝图调用。
	 *
	 * 失败返回：未配置或加载失败时返回 nullptr，且**不打警告**（名单是可选的，缺了不影响战斗）。
	 *          注意「没配」和「配了但列表为空」都体现为 nullptr / Num()==0，调用方两者都要能处理。
	 *
	 * @param WorldContextObject 未参与查找，仅为蓝图节点的世界上下文占位。
	 */
	UFUNCTION(BlueprintPure, Category = "Aura|Character", meta = (WorldContext = "WorldContextObject"))
	static const UAuraCharacterRoster* GetCharacterRoster(const UObject* WorldContextObject);
};
