// Study Game Ability System _Hades

/**
 * @file AuraPlayerController.h
 * @brief 玩家控制器声明 —— 处理鼠标光标高亮追踪与 WASD 移动输入。
 *
 * 职责：
 * - PlayerTick 中每帧调用 CursorTrace()，通过碰撞检测追踪光标下方的敌人，自动调用高亮/取消高亮接口。
 * - 使用 Enhanced Input 系统将 MoveAction 绑定到 Move()，实现基于控制器朝向的世界空间移动。
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class UFloatingTextWidgetComponent;
class ACharacter;
class UAuraAbilitySystemComponent;
struct FGameplayTag;
class UAuraInputConfig;
class IEnemyInterface;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 玩家控制器 —— Enhanced Input 驱动的 WASD/摇杆移动 + 光标敌方高亮
 * 网络：bReplicates = true，可被复制到远程客户端
 * 输入：Enhanced Input → MoveAction → Move()
 * 高亮：每帧 CursorTrace() 投射射线，在 IEnemyInterface 上调用 HighlightActor / UnHighlightActor
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AAuraPlayerController();

	/** 每帧驱动 CursorTrace 以更新光标下方目标的高亮状态 */
	virtual void PlayerTick(float DeltaTime) override;

	/**
	 * 伤害飘字 —— 只在本机客户端创建，服务器与其它玩家都不显示。
	 *
	 * 为什么必须声明成 Client RPC：
	 *  - 调用点（UAuraAttributeSet::PostGameplayEffectExecute、
	 *    UExecutionCalculation_Damage::Execute_Implementation）只在服务器执行，
	 *    服务器只负责"算伤害 / 判定暴击闪避"，不负责"画字"；
	 *  - 普通 C++ 函数会在服务器进程里 NewObject，组件只存在于服务器世界，
	 *    远端客户端的屏幕层永远收不到——这就是"listen host 正常、客户端没有"的根因；
	 *  - 改成 Client RPC 后，网络层只把这次调用发给该 PlayerController 的拥有者客户端，
	 *    真正的 NewObject 发生在"这次攻击所属的那个客户端"，因此每个玩家只在自己屏幕上
	 *    看到自己造成的伤害，专用服务器上一个飘字都不会创建。
	 *
	 * 注意：listen host 调用 Client RPC 时 Callspace 为 Local，会就地执行 _Implementation，
	 * 所以 host 作为玩家仍能看到自己的伤害数字；但不会再看到远端玩家的伤害数字。
	 * 本函数只能由服务器调用：客户端没有权威的伤害数值，也不该主动调它。
	 *
	 * @param InDamage        本次伤害的最终数值（暴击倍率已乘好；闪避时为 0）
	 * @param TargetCharacter 飘字挂载的目标角色（被打的敌人），同时作为组件 Outer
	 * @param bIsDodge		  攻击是否被闪避
	 * @param bIsCriticalHit  攻击是否暴击
	 *
	 * 注意：这里刻意只传 float + 两个 bool，不再传 FText / 颜色 / 字号 ——
	 * 文案、颜色、字号、动画都属于纯表现，统一交给蓝图的 SetFloatingText 事件处理；
	 * 参数全是 POD，RPC 序列化最省，也顺带避开了 UHT 的一个坑：RPC 的 exec thunk
	 * 形参会被改写成 `const T&`，声明里写 `FText Text`（按值）会生成对不上的 thunk，报 C2511。
	 */
	UFUNCTION(Client, Reliable)
	void ShowFloatingText(float InDamage, ACharacter* TargetCharacter, bool bIsDodge = false, bool bIsCriticalHit = false);
protected:
	
	/** 配置鼠标光标、输入模式（Game & UI）、注册 AuraContext 映射上下文 */
	virtual void BeginPlay() override;

	/** 将 MoveAction 绑定到 Enhanced Input Component 的 Triggered 阶段 */
	virtual void SetupInputComponent() override;
	
private:
	/**
	 * 移动回调 —— 由 Enhanced Input 在 Triggered 阶段持续调用
	 * @param Value 包含二维输入向量（左右 = X，前后 = Y）
	 */
	void Move(const FInputActionValue &Value);

	/**
	 * 每帧从光标位置投射 Visibility 射线，检测下方 Actor 是否实现了 IEnemyInterface，
	 * 并据此切换高亮状态。详见 .cpp 中的 4 种情况表。
	 */
	void CursorTrace();

	/**
	 * 能力输入回调（由 BindAbilityActions 绑定，携带对应 GameplayTag）。
	 * 三者在配置中的每个输入动作上分别绑定到 Started / Completed / Triggered 事件。
	 */
	void AbilityInputTagPressed(FGameplayTag InputTag);   // 按下（Started）
	void AbilityInputTagReleased(FGameplayTag InputTag);  // 松开（Completed）
	void AbilityInputTagHeld(FGameplayTag InputTag);      // 按住（Triggered）
	
	UAuraAbilitySystemComponent* GetASC();
	
protected:
	/** Enhanced Input 映射上下文（含 MoveAction 等输入操作） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	/** WASD / 摇杆移动输入操作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	/** 输入配置数据资产：定义「输入动作 ↔ GameplayTag」映射，供 BindAbilityActions 批量绑定能力输入 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> AuraInputConfig;
	
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UFloatingTextWidgetComponent> FloatingTextWidgetClass;

private:
	
	//IEnemyInterface 是原生 C++ 接口类，所以必须用原始指针 IEnemyInterface*
	/** 上一帧光标下方的 IEnemyInterface Actor（用于高亮状态切换对比） */
	IEnemyInterface* LastActor = nullptr;

	/** 当前帧光标下方的 IEnemyInterface Actor */
	IEnemyInterface* CurrentActor = nullptr;
}; 