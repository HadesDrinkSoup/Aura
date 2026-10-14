// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"		// UAbilityTask 基类：Activate()/EndTask()/ShouldBroadcastAbilityTaskDelegates() 等
#include "TargetDataUnderMouse.generated.h"	// UHT 生成代码，必须放在所有 include 的最后一行

/**
 * 任务的"输出口"：动态多播委托（蓝图可绑定的动态委托，参数必须用 USTRUCT/UObject 类型）。
 * FGameplayAbilityTargetDataHandle 定义在 Abilities/GameplayAbilityTargetTypes.h:202。
 *   - 本地控制端：SendMouseCursorData() 采集完数据后立刻广播（预测，零延迟手感）。
 *   - 服务器/代理端：OnTargetDataReplicatedCallback() 收到客户端 RPC 后广播。
 * 蓝图里在节点的 "Assign Valid Data" 引脚上绑定事件，就能拿到鼠标指向的目标数据。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataDelegate, const FGameplayAbilityTargetDataHandle&, Data);

/**
 * UTargetDataUnderMouse —— 自定义 AbilityTask：采集"鼠标光标下的目标信息"并交给技能使用。
 * 项目中由 GA_FireBall 蓝图调用（所以 meta 里标了 BlueprintInternalUseOnly）。
 *
 * 一个 AbilityTask 的标准三件套（见 AbilityTask.h:28-44 的官方注释）：
 *   1) 动态多播委托：任务的输出（本类是 ValidData）。
 *   2) static 工厂函数：任务的输入，只负责 new 出实例并注册进技能，绝不在这里执行逻辑/广播委托。
 *   3) override Activate()：真正的执行入口，本类就在这里"开始干活"。
 *
 * 网络职责划分（整个类的核心思想）：
 *   - 拥有者客户端 IsLocallyControlled()==true ：鼠标在本机，自己采集数据 → 本地广播给技能 + 把数据发给服务器。
 *   - 服务器 / 非本地代理    ==false：自己没有鼠标，只能注册回调，等客户端把数据同步过来后再广播。
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
public:
	
	/**
	 * 静态工厂函数 = 蓝图节点 "TargetDataUnderMouse"。
	 * meta 参数说明：
	 *   DisplayName              - 蓝图节点显示名
	 *   HidePin="OwningAbility"  - 隐藏 OwningAbility 引脚，不暴露给策划
	 *   DefaultToSelf            - 该引脚不连时默认取当前技能自身（技能蓝图里就是 self）
	 *   BlueprintInternalUseOnly - 只能在蓝图内部被 Latent 节点调用（不要从别的 C++ 手工调用）
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);
	
	/** 任务的输出：拿到有效目标数据时广播（蓝图 "Assign Valid Data" 绑定到这里）。 */
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataDelegate ValidData;
	
protected:
	/** 任务的真正入口：由 ReadyForActivation() 触发（蓝图 Latent 节点会自动调用它）。 */
	virtual void Activate() override;
	
private:
	
	/** 本地控制端：用鼠标光标下的命中结果构造目标数据，广播给技能，并同步给服务器。 */
	void SendMouseCursorData() const;
	/** 服务器/代理端：客户端的复制数据到达时回调；也用于处理"数据先到、任务后激活"的竞态。 */
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag) const;
};
