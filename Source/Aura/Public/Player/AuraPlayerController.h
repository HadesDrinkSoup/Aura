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

	/** 伤害飘字 —— 只在本地客户端执行：由本地客户端上的 GC_Damage 直接调用，在目标角色身上创建浮动数字 */
	UFUNCTION(BlueprintCallable, Category = "Aura|PlayerController")
	void ShowFloatingText(float InDamage, AActor* TargetActor, const bool bIsDodge, const bool bIsCriticalHit);
	
	/**
	 * 取当前光标下方的敌人 Actor（CursorTrace 每帧维护的 CurrentActor）。
	 * 对外只暴露成 AActor*：IEnemyInterface 不是 UObject 派生，蓝图侧无法直接持有，
	 * 调用方需要时自行 Cast 到具体敌人类型（如 AAuraEnemy）再取血条控制器。
	 * 返回 nullptr 的情形：光标下没有 Actor、命中的 Actor 没有实现 IEnemyInterface，
	 * 或在本帧的射线没有遮挡命中（CursorTrace 会提前 return，CurrentActor 保持上一帧值）。
	 */
	UFUNCTION(BlueprintPure, Category = "Aura|Player|Cursor", meta = (DisplayName = "获取光标下的敌人"))
	AActor* GetCursorTargetActor() const;
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aura|Player|Input", meta = (DisplayName = "输入映射上下文"))
	TObjectPtr<UInputMappingContext> AuraContext;

	/** WASD / 摇杆移动输入操作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aura|Player|Input", meta = (DisplayName = "移动输入动作"))
	TObjectPtr<UInputAction> MoveAction;
	
	/** 输入配置数据资产：定义「输入动作 ↔ GameplayTag」映射，供 BindAbilityActions 批量绑定能力输入 */
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Player|Input", meta = (DisplayName = "输入配置"))
	TObjectPtr<UAuraInputConfig> AuraInputConfig;
	
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Aura|Player|UI", meta = (DisplayName = "浮动文字控件类"))
	TSubclassOf<UFloatingTextWidgetComponent> FloatingTextWidgetClass;

private:
	
	//IEnemyInterface 是原生 C++ 接口类，所以必须用原始指针 IEnemyInterface*
	/** 上一帧光标下方的 IEnemyInterface Actor（用于高亮状态切换对比） */
	IEnemyInterface* LastActor = nullptr;

	/** 当前帧光标下方的 IEnemyInterface Actor */
	IEnemyInterface* CurrentActor = nullptr;
}; 