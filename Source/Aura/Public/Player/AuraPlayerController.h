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
	void Move(const FInputActionValue& Value);

	/**
	 * 每帧从光标位置投射 Visibility 射线，检测下方 Actor 是否实现了 IEnemyInterface，
	 * 并据此切换高亮状态。详见 .cpp 中的 4 种情况表。
	 */
	void CursorTrace();


protected:
	/** Enhanced Input 映射上下文（含 MoveAction 等输入操作） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	/** WASD / 摇杆移动输入操作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

private:
	//IEnemyInterface 是原生 C++ 接口类，所以必须用原始指针 IEnemyInterface*
	/** 上一帧光标下方的 IEnemyInterface Actor（用于高亮状态切换对比） */
	IEnemyInterface* LastActor = nullptr;

	/** 当前帧光标下方的 IEnemyInterface Actor */
	IEnemyInterface* CurrentActor = nullptr;
}; 