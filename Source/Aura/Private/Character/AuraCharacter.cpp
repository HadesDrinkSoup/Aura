// Study Game Ability System _Hades

/**
 * @file AuraCharacter.cpp
 * @brief 玩家角色实现 —— 配置 CharacterMovement 和旋转行为。
 *
 * 该类继承自 ACharacter，代表玩家在游戏世界中操控的角色实体。
 * 主要职责：
 * - 在构造函数中配置移动组件（朝向旋转、平面约束）
 * - 禁用控制器旋转影响，使角色朝向完全由移动方向决定
 */

#include "Character/AuraCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

// -----------------------------------------------------------------------------
// 构造
// -----------------------------------------------------------------------------

AAuraCharacter::AAuraCharacter()
{
	// --- 移动朝向 ---
	// 移动时自动将角色旋转至移动方向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 设置旋转速率：Yaw 每秒 400°，Pitch / Roll 无旋转
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

	// --- 平面约束 ---
	// 将角色移动约束在一个平面上（适用于自上而下 / 等距视角游戏）
	GetCharacterMovement()->bConstrainToPlane = true;
	// 游戏开始时将角色吸附到平面，避免因初始位置不在平面上导致的穿透或抖动
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// --- 控制器旋转 ---
	// 禁用控制器 Pitch / Yaw / Roll 对角色旋转的影响
	// 角色的朝向完全由移动方向决定，而非摄像机方向
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitActorInfo();
	AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitActorInfo();
}

int32 AAuraCharacter::GetCharacterLevel()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetCharacterLevel();
}

void AAuraCharacter::InitActorInfo()
{
	if (bActorInfoInitialized) return;

	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	if (AuraPlayerState == nullptr) return;

	AuraAbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AuraAbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);
	AuraAttributeSet = AuraPlayerState->GetAttributeSet();
	InitDefaultAttributes();

	bActorInfoInitialized = true;
	
	// 初始化 UI
	if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			const FWidgetControllerParams Params(PC, AuraPlayerState, AuraAbilitySystemComponent, AuraAttributeSet);
			HUD->InitOverlay(Params);
		}
	}
}
