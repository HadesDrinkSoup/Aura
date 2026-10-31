// Study Game Ability System _Hades

/**
 * @file AuraEnemy.cpp
 * @brief 敌方角色实现 —— 配置 Visibility 碰撞通道以响应光标射线检测，并实现高亮/取消高亮。
 */

#include "Character/AuraEnemy.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Aura/Aura.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/WidgetController/EnemyHealthBarWidgetController.h"

// -----------------------------------------------------------------------------
// 构造
// -----------------------------------------------------------------------------

AAuraEnemy::AAuraEnemy()
{
	// 将骨骼网格的 Visibility 通道设置为 Block，使 AuraPlayerController::CursorTrace()
	// 中的 GetHitResultUnderCursor(ECC_Visibility) 能够命中该敌人
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	AuraAbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AuraAbilitySystemComponent->SetIsReplicated(true);
	AuraAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AuraAttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitActorInfo();
}

// -----------------------------------------------------------------------------
// IEnemyInterface 实现 —— 基于 Custom Depth + Stencil 的高亮
// -----------------------------------------------------------------------------

void AAuraEnemy::InitActorInfo()
{
	AuraAbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitDefaultAttributes();
	InitHealthBar();
}

void AAuraEnemy::InitHealthBar()
{
	// HealthBarControllerClass 是 TSubclassOf（类引用），不是实例：
	// 必须先 NewObject 创建出 UEnemyHealthBarWidgetController 实例，再对它调用成员函数。
	if (!HealthBarControllerClass) return;

	UAuraUserWidget* AuraUserWidget = HealthBar ? Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()) : nullptr;
	if (!AuraUserWidget) return;

	// 每个敌人持有独立的 Controller 实例，Outer 设为 this，生命周期随敌人
	HealthBarController = NewObject<UEnemyHealthBarWidgetController>(this, HealthBarControllerClass);

	// 敌人血条只用到自身的 ASC / AttributeSet；
	// PC / PS 仅用于填充参数结构体（无玩家时允许为空，此 Controller 不会访问它们）
	AAuraPlayerController* PC = GetWorld() ? Cast<AAuraPlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
	AAuraPlayerState* PS = PC ? Cast<AAuraPlayerState>(PC->PlayerState) : nullptr;

	HealthBarController->SetWidgetControllerParams(FWidgetControllerParams(PC, PS, AuraAbilitySystemComponent, AuraAttributeSet));

	// 顺序：先设置参数 → 绑定 Widget → 绑定委托回调 → 广播初始值
	AuraUserWidget->SetWidgetController(Cast<UObject>(HealthBarController));
	HealthBarController->BindCallbacksToDependencies();
	HealthBarController->BroadCastInitialValues();
}

void AAuraEnemy::HighlightActor()
{
	// 开启 Custom Depth 渲染，写入红色模板值，供后处理描边材质读取
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

	// 武器组件同步高亮，确保与角色主体视觉一致
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnemy::UnHighlightActor()
{
	// 关闭 Custom Depth 渲染即可移除描边 —— 模板值无需重置
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

int32 AAuraEnemy::GetCharacterLevel()
{
	return Level;
}
