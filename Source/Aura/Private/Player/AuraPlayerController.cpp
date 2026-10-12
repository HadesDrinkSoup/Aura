// Study Game Ability System _Hades

/**
 * @file AuraPlayerController.cpp
 * @brief 玩家控制器实现 —— 处理玩家输入并将移动指令转发给受控 Pawn。
 *
 * 该类继承自 APlayerController，使用 Enhanced Input 系统绑定输入操作。
 * 主要职责：
 * - 在 BeginPlay 中配置鼠标光标和输入模式（Game & UI）
 * - 注册 AuraContext（输入映射上下文）
 * - 在 SetupInputComponent 中将 MoveAction 绑定到 OnMove 回调
 * - OnMove 负责根据控制器的 Yaw 朝向将二维输入向量转化为世界空间移动
 */

#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Engine/HitResult.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "Input/AuraEnhancedInputComponent.h"
#include "Interface/EnemyInterface.h"
#include "Player/AuraPlayerState.h"

// -----------------------------------------------------------------------------
// 构造 & 初始化
// -----------------------------------------------------------------------------

AAuraPlayerController::AAuraPlayerController()
{
	// 启用网络复制，使该 PlayerController 在多人环境下能被同步到其他客户端
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(const float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// --- 光标 & 输入模式 ---
	// 显示鼠标光标，使玩家可以同时与 UI 和游戏世界交互
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	// 不将鼠标锁定到视口 —— 允许鼠标自由移出窗口
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 鼠标捕获期间仍显示光标，避免光标突然消失
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
  
	// --- 注册 Enhanced Input 映射上下文 ---
	// 通过 LocalPlayer 获取 EnhancedInput 子系统，并将 AuraContext 映射上下文添加到最高优先级
	// 注意：每个 LocalPlayer 有自己独立的 EnhancedInput 子系统，因此客户端各自注册即可
	if (const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (AuraContext)
			{
				// 优先级 0 表示最高优先级（数字越小优先级越高）
				Subsystem->AddMappingContext(AuraContext, 0);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// 输入绑定
// -----------------------------------------------------------------------------

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 将输入组件转换为 EnhancedInputComponent 以支持 Enhanced Input 绑定
	if (UAuraEnhancedInputComponent* AuraEnhancedInput = CastChecked<UAuraEnhancedInputComponent>(InputComponent))
	{
		// 绑定 MoveAction：在 Triggered 阶段持续调用 OnMove，实现按住方向键/WASD 时的连续移动
		AuraEnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
		AuraEnhancedInput->BindAbilityActions(AuraInputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	}
}

// -----------------------------------------------------------------------------
// 输入回调
// -----------------------------------------------------------------------------

void AAuraPlayerController::Move(const FInputActionValue& Value)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		// 从 InputActionValue 中提取二维移动向量
		// X = 水平分量（左右），Y = 垂直分量（前后）
		const FVector2D MovementVector = Value.Get<FVector2D>();

		// 基于控制器的 Yaw 旋转构建世界空间方向向量
		// 仅使用 Yaw（忽略 Pitch 和 Roll），保证移动始终在水平面上
		const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

		// 从旋转矩阵中提取前向和右向单位轴
		// X 轴 = Forward，Y 轴 = Right
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 将输入分量分别沿 Forward 和 Right 轴施加到受控 Pawn
		// MovementVector.Y = 前后输入（W/S 或摇杆上下）→ 沿 Forward 移动
		// MovementVector.X = 左右输入（A/D 或摇杆左右）→ 沿 Right  移动
		ControlledPawn->AddMovementInput(Forward, MovementVector.Y);
		ControlledPawn->AddMovementInput(Right, MovementVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	// 每帧从光标位置投射射线，检测下方是否有实现了 IEnemyInterface 的 Actor
	FHitResult CursorHitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHitResult);
	if (!CursorHitResult.bBlockingHit) return;
	
	// 帧状态：保存上一帧命中的 Actor，更新当前帧命中
	LastActor = CurrentActor;
	CurrentActor = Cast<IEnemyInterface>(CursorHitResult.GetActor());
	
	// --- 高亮状态切换逻辑 ---
	// 仅当目标发生变化时才更新高亮：先取消旧目标，再高亮新目标
	if (CurrentActor != LastActor)
	{
		if (LastActor != nullptr)
		{
			LastActor->UnHighlightActor();
		}
		if (CurrentActor != nullptr)
		{
			CurrentActor->HighlightActor();
		}
	}
}

// -----------------------------------------------------------------------------
// 能力输入回调
// 由 BindAbilityActions 在 SetupInputComponent 中绑定，分别对应输入动作的
// Started / Completed / Triggered 阶段。这里只负责「输入事件 → ASC」的转发，
// 真正的能力激活 / 触发逻辑在 UAuraAbilitySystemComponent 中处理。
// -----------------------------------------------------------------------------

void AAuraPlayerController::AbilityInputTagPressed(const FGameplayTag InputTag)
{
	// 按下（Started）→ 转发给 ASC，处理能力的「按下」事件
	GetASC()->AbilityInputTagPressed(InputTag);
}

void AAuraPlayerController::AbilityInputTagReleased(const FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	// 松开（Completed）→ 转发给 ASC，处理能力的「松开」事件（如蓄力技能施放）
	GetASC()->AbilityInputTagReleased(InputTag);
}

void AAuraPlayerController::AbilityInputTagHeld(const FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	// 按住（Triggered，每帧触发）→ 转发给 ASC，负责激活尚未激活的能力
	GetASC()->AbilityInputTagHeld(InputTag);
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	// 懒加载缓存：首次调用时借道 PlayerState 获取 ASC，之后直接返回缓存指针
	if (AuraAbilitySystemComponent == nullptr)
	{
		// ASC 由 AAuraPlayerState 持有（见其构造函数），PlayerController 仅缓存引用
		if (const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
		{
			AuraAbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
		}
	}
	return AuraAbilitySystemComponent;
}
