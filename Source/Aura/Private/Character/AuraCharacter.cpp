// Study Game Ability System _Hades

/**
 * @file AuraCharacter.cpp
 * @brief 玩家角色实现 —— 移动/旋转配置，以及 GAS 的玩家侧初始化链路。
 *
 * 该类继承自 AAuraCharacterBase，代表玩家操控的角色实体。主要职责：
 * - 构造函数中配置移动组件（朝向跟随移动、平面约束、屏蔽控制器旋转）；
 * - 从 AAuraPlayerState 借 ASC / AttributeSet 并完成 InitActorInfo（服务器与客户端两条路径）；
 * - 在 PawnClientRestart 中（Controller 就绪后）创建 Overlay UI。
 */

#include "Character/AuraCharacter.h"
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

/**
 * 服务器侧初始化入口：控制器接管 Pawn 时触发。
 * 必须放在 Super::PossessedBy 之后 —— 父类内部才会把 PlayerState 关联到 Pawn，
 * 否则 InitActorInfo 里 GetPlayerState 拿到的是空指针。
 * 这里额外调用 AddCharacterAbilities()，因为技能授予只需在服务器执行一次。
 */
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitActorInfo();
	AddCharacterAbilities();
}

/**
 * 客户端侧初始化入口：PlayerState 复制到达时触发。
 * 与 PossessedBy 的时序不保证谁先，因此同样调用 InitActorInfo（由幂等标志去重）。
 * 注意此处不能初始化 UI：此时 Controller 可能尚未复制到客户端。
 */
void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitActorInfo();
}

/**
 * 返回玩家等级，数据源是 AAuraPlayerState::GetCharacterLevel()。
 * 用 check 而不是判空返回 1：玩家角色一定挂在 AuraPlayerState 下，
 * 取不到说明 DefaultPawn/PlayerStateClass 配错，属于开发期错误，应当立即暴露。
 */
int32 AAuraCharacter::GetCharacterLevel()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetCharacterLevel();
}

/**
 * GAS 初始化：
 *  1. 幂等判断 + PlayerState 可用性判断（客户端 OnRep 可能先于 PlayerState 就绪）；
 *  2. ASC 取自 PlayerState，并调用 InitAbilityActorInfo(OwnerActor=PlayerState, AvatarActor=this)，
 *     这是玩家侧 ASC 的标准做法：ASC 常驻 PlayerState，Avatar 换角色时不丢失状态；
 *  3. 加载默认属性（基类实现，内部把 GE 应用在 ASC 上，只有服务器生效）。
 * 结束后置位 bActorInfoInitialized，避免另一条路径重复执行。
 */
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
	
	// UI 初始化不在这里做：客户端 OnRep_PlayerState 触发时 Controller 可能尚未复制，
	// 统一移到 PawnClientRestart（Controller 保证就绪）中。
}

/**
 * Pawn 被本地玩家接管后触发（服务器与本地客户端都会走）。
 * 放在这里而不是 InitActorInfo 的原因：只有此时 Controller 一定已就绪，
 * 才能 Cast 出 AAuraPlayerController 并取到它的 HUD。
 */
void AAuraCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	// 本地玩家 Possess 完成后触发，此时 Controller / PlayerState 均已就绪
	InitOverlayWidget();
}

/**
 * 创建并初始化 Overlay（HUD 上的血条/蓝条等）。
 * 任何一环缺失都直接返回，属于"还没准备好"，不是错误：
 *  - GAS 未初始化：等 PawnClientRestart 对应的下一次时机；
 *  - 没有玩家控制器（例如服务器上的 Pawn）：服务器不需要 Overlay。
 * Params 把 PC/PS/ASC/AS 一次性交给 HUD，由 HUD 决定用哪套 WidgetController。
 */
void AAuraCharacter::InitOverlayWidget() const
{
	// 确保 GAS 已初始化、Controller 已就绪
	if (!bActorInfoInitialized) return;

	AAuraPlayerController* PC = Cast<AAuraPlayerController>(GetController());
	if (!PC) return;

	if (AAuraHUD* HUD = Cast<AAuraHUD>(PC->GetHUD()))
	{
		AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
		const FWidgetControllerParams Params(PC, AuraPlayerState, AuraAbilitySystemComponent, AuraAttributeSet);
		HUD->InitOverlay(Params);
	}
}
