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
	// 4 种情况：
	//   Last=null,  Current=null  → 无操作
	//   Last=null,  Current=A     → 高亮 A（光标首次进入敌人）
	//   Last=A,     Current=null  → 取消高亮 A（光标离开敌人）
	//   Last=A,     Current=A     → 无操作（仍在同一敌人上）
	//   Last=A,     Current=B     → 取消 A，高亮 B（光标从一个敌人移到另一个）
	
	if (LastActor == nullptr)
	{
		if (CurrentActor != nullptr)
		{
			// 光标首次悬停到敌人上
			CurrentActor->HighlightActor();
		}
	}
	else  // LastActor != nullptr —— 上一帧光标下方有敌人
	{
		if (CurrentActor != nullptr)
		{
			// 光标仍在一个敌人上，检查是否切换了目标
			if (LastActor != CurrentActor)
			{
				LastActor->UnHighlightActor();
				CurrentActor->HighlightActor();
			}
			// 否则 LastActor == CurrentActor，同一目标，无需操作
		}
		else  // CurrentActor == nullptr —— 光标已移开，不再指向任何敌人
		{
			LastActor->UnHighlightActor();
		}
	}
}

// -----------------------------------------------------------------------------
// 能力输入回调
// 由 BindAbilityActions 在 SetupInputComponent 中绑定，
// 分别对应输入动作的 Started / Completed / Triggered 阶段。
// 当前仅打印对应 GameplayTag 用于调试，后续将在此触发 GAS 能力。
// -----------------------------------------------------------------------------

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	// 按下（Started）—— 绿色提示
	GEngine->AddOnScreenDebugMessage(1, 5, FColor::Green, InputTag.ToString());
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	// 松开（Completed）—— 蓝色提示
	GEngine->AddOnScreenDebugMessage(2, 5, FColor::Blue, InputTag.ToString());
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	// 按住（Triggered）—— 红色提示
	GEngine->AddOnScreenDebugMessage(3, 5, FColor::Red, InputTag.ToString());
}