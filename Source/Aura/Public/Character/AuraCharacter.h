// Study Game Ability System _Hades

/**
 * @file AuraCharacter.h
 * @brief 玩家角色 —— 组装 GAS 的"玩家侧"初始化链路（ASC 放在 PlayerState 上）。
 *
 * 与 AAuraEnemy 的关键差异：本类不自己创建 ASC / AttributeSet，而是从
 * AAuraPlayerState 借用，因此初始化天然分成两条可能"迟到"的路径：
 *  - 服务器：PossessedBy()        → InitActorInfo()
 *  - 客户端：OnRep_PlayerState()  → InitActorInfo()
 * 二者谁先发生取决于网络时序，所以两条路径都调用 InitActorInfo，
 * 由 bActorInfoInitialized 保证只真正执行一次。
 * UI 初始化不能放在这两处（客户端此时 Controller 可能还没复制），
 * 统一推迟到 PawnClientRestart()。
 */

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"

/**
 * 玩家可控角色。
 *
 * 职责有两块：
 *  1. 移动手感：构造时把 CharacterMovement 配成"朝向跟随移动方向"的等距/第三人称风格；
 *  2. GAS 装配：从 PlayerState 借 ASC，初始化 ActorInfo 与默认属性，并在合适的时机拉起 HUD。
 * 属性数值本身不在这里定义，统一由基类 InitDefaultAttributes() 按职业表加载。
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()
public:
	/**
	 * 配置移动与旋转行为：
	 * - bOrientRotationToMovement = true：角色朝向跟随移动方向，而非摄像机；
	 * - RotationRate 只保留 Yaw，避免角色在平面上被 Pitch/Roll 带偏；
	 * - bConstrainToPlane + bSnapToPlaneAtStart：把角色约束在固定平面，并在开始时吸附上去；
	 * - bUseControllerRotation* 全部关闭：否则控制器旋转会覆盖上面的朝向设置。
	 */
	AAuraCharacter();
	
	/**
	 * 服务器在"控制器接管该 Pawn"时调用（只在服务器执行）。
	 * 顺序不能反：先 Super::PossessedBy（此时 Pawn 才真正拿到 PlayerState），
	 * 再 InitActorInfo（内部会 GetPlayerState），最后授予技能。
	 * 客户端不会进入这里，对应逻辑由 OnRep_PlayerState 承担。
	 */
	virtual void PossessedBy(AController* NewController) override;
	/**
	 * 客户端在 PlayerState 复制到达时调用（只在客户端执行）。
	 * 与 PossessedBy 是"谁先到不确定"的关系，因此同样走 InitActorInfo，
	 * 其内部的幂等标志会过滤掉重复初始化。
	 */
	virtual void OnRep_PlayerState() override;
	/**
	 * 本地玩家 Pawn 重启（重新 Possess）时调用，此时 Controller 已经就绪。
	 * 这是初始化 Overlay UI 的正确时机：OnRep_PlayerState 阶段 Controller 可能尚未复制，
	 * 拿不到 AuraPlayerController / HUD。
	 */
	virtual void PawnClientRestart() override;
	
	/** 玩家等级存放在 PlayerState（跨死亡保留、且会被复制到客户端）；check 用于暴露 PlayerState 类型配错 */
	virtual int32 GetCharacterLevel() override;

protected:
	/**
	 * 从 AAuraPlayerState 取得 ASC / AttributeSet，初始化 ActorInfo 并加载默认属性。
	 * 幂等：已初始化或 PlayerState 尚未复制时直接返回，等待下一次回调。
	 */
	virtual void InitActorInfo() override;
	/** 通过 HUD 创建 Overlay（血条、蓝条等）；前提是 Controller、PlayerState、ASC 都已就绪 */
	void InitOverlayWidget() const;

	/** 防止 PossessedBy 和 OnRep_PlayerState 重复初始化 */
	bool bActorInfoInitialized = false;
};
