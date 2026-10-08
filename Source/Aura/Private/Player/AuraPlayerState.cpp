// Study Game Ability System _Hades

#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Net/UnrealNetwork.h"

/**
 * @brief AuraPlayerState 构造函数
 * 
 * PlayerState 作为玩家在网络游戏中的持久状态载体，在服务端和客户端之间同步。
 * 这里将其作为 GAS（Gameplay Ability System）的核心持有者：
 * - 持有 AbilitySystemComponent（技能系统组件），负责技能的授予、激活与 GameplayEffect 的管理
 * - 持有 AttributeSet（属性集），存储玩家的各项属性值（生命、魔力等）
 * 
 * 将 ASC 放在 PlayerState 上（而非 Pawn 或 PlayerController）的好处：
 * - PlayerState 在死亡/重生时不销毁，属性与技能不会丢失
 * - 天然支持网络复制，所有客户端都能收到状态同步
 */
AAuraPlayerState::AAuraPlayerState()
{
	// 设置网络更新频率为 100Hz，确保属性变化能及时同步到所有客户端
	// 对于 ARPG 类型的游戏，较高的更新频率能减少属性（血量等）的延迟感
	SetNetUpdateFrequency(100.f);
	
	// 创建技能系统组件（ASC），这是 GAS 的核心，所有 GameplayAbility 和 GameplayEffect 都依赖它
	AuraAbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	// 将 ASC 标记为可复制，使其在网络中同步
	AuraAbilitySystemComponent->SetIsReplicated(true);
	// 使用 Mixed 复制模式：
	// - GameplayEffect 对所有玩家复制（用于 UI 显示如血量条）
	// - GameplayTag 只对拥有者复制（减少带宽）
	// - 这是多人 ARPG 推荐使用的模式
	AuraAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	// 创建属性集，存储所有 GameplayAttribute（生命值、魔力值、攻击力等）
	AuraAttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AAuraPlayerState, Level, COND_None, REPNOTIFY_OnChanged);
}

void AAuraPlayerState::OnRep_Level(const int32 OldLevel)
{
}
