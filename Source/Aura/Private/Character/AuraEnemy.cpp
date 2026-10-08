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
