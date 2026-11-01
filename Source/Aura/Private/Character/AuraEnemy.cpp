// Study Game Ability System _Hades

/**
 * @file AuraEnemy.cpp
 * @brief 敌方角色实现 —— 自建 ASC/AttributeSet、配置光标检测碰撞、实现高亮与头顶血条。
 *
 * 与玩家角色的差异：敌人的 ASC 与 AttributeSet 都由本对象用 CreateDefaultSubobject 创建，
 * 复制模式取 Minimal（客户端只需表现数据，不需要完整的 GE 信息）。
 */

#include "Character/AuraEnemy.h"

#include "AuraGamepalyTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Aura/Aura.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
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

/**
 * BeginPlay：服务器与客户端都会执行。
 * 直接调用 InitActorInfo，其中 InitAbilityActorInfo(this, this) 会按网络角色
 * 自行决定初始化 Owner/Avatar 的哪一侧，因此无需在这里判断 HasAuthority()。
 */
void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	
	InitActorInfo();
	
	AuraAbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact,
		EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AAuraEnemy::HitReactTagChanged); 
}

// -----------------------------------------------------------------------------
// IEnemyInterface 实现 —— 基于 Custom Depth + Stencil 的高亮
// -----------------------------------------------------------------------------

/**
 * 敌人在自身身上完成 GAS 初始化：
 *  1. InitAbilityActorInfo(this, this)：Owner 与 Avatar 都是敌人自己；
 *  2. InitDefaultAttributes()：按职业表应用初始属性 GE（非服务器会直接返回）；
 *  3. InitHealthBar()：创建头顶血条控制器。
 * 最后一步依赖前两步——血条要读 ASC/AS，所以顺序不能调整。
 */
void AAuraEnemy::InitActorInfo()
{
	AuraAbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitDefaultAttributes();
	// 敌人同样需要授予默认技能：受击反应（GA_HitReact）挂在职业表的 CommonAbilities 上，
	// 不授予的话 PostGameplayEffectExecute 里的 TryActivateAbilitiesByTag 找不到任何可激活技能，
	// 受击蒙太奇不会播放（且 Effects.HitReact 标签也不会被加上，敌人不会停步）。
	// 授予动作内部有 HasAuthority 判断，客户端调用无副作用。
	AddCharacterAbilities();
	InitHealthBar();
}

void AAuraEnemy::InitHealthBar()
{
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

	// 兜底：无论初始值何时就绪，未战斗时血条必须是隐藏的。
	// 蓝图 PreConstruct 里已经设过一次 Hidden，但客户端 BeginPlay 与属性复制存在时序竞争，
	// 这里在初始化结束时再显式隐藏一次，保证启动瞬间不会残留可见血条。
	AuraUserWidget->SetVisibility(ESlateVisibility::Hidden);
}

/**
 * 被 AuraPlayerController::CursorTrace() 在"光标刚移入该敌人"时调用一次。
 * 角色本体与武器分开设置：Weapon 是独立的骨骼网格组件，不跟随 GetMesh 的渲染开关。
 * 模板值（Stencil）只需在开启时写入，关闭时不必重置——后处理材质只在 CustomDepth 开启时采样。
 */
void AAuraEnemy::HighlightActor()
{
	// 开启 Custom Depth 渲染，写入红色模板值，供后处理描边材质读取
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

	// 武器组件同步高亮，确保与角色主体视觉一致
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

/**
 * 光标移出该敌人时调用，与 HighlightActor 成对；
 * 若这里漏关，敌人会一直保持描边（高亮残留）。
 */
void AAuraEnemy::UnHighlightActor()
{
	// 关闭 Custom Depth 渲染即可移除描边 —— 模板值无需重置
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

/** 敌人等级直接读自身配置，不涉及跨对象查询，因此没有 check/回退逻辑 */
int32 AAuraEnemy::GetCharacterLevel()
{
	return Level;
}

void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReact = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReact ? 0.f : BaseWalkSpeed;
}
