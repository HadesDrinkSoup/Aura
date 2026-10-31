// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AuraWidgetController.generated.h"

class UAuraAttributeSet;
class UAuraAbilitySystemComponent;
class AAuraPlayerState;
class AAuraPlayerController;

/** 浮点属性值变化委托：NewValue — 变化后的属性值，OldValue — 变化前的属性值 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeValueChangedDelegate, float, NewValue, float, OldValue);


/**
 * WidgetController 初始化参数结构体
 *
 * 将 GAS 所需的四个核心对象打包传递给 WidgetController。
 * 设计为结构体而非散列参数，便于扩展且避免参数列表过长。
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {}

	FWidgetControllerParams(AAuraPlayerController* PC, AAuraPlayerState* PS, UAuraAbilitySystemComponent* ASC, UAuraAttributeSet* AS)
	{
		AuraPlayerController = PC;
		AuraPlayerState = PS;
		AuraAbilitySystemComponent = ASC;
		AuraAttributeSet = AS;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AAuraPlayerController> AuraPlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AAuraPlayerState> AuraPlayerState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;
};

/**
 * WidgetController 抽象基类
 *
 * 作为 UI 层与 GAS（Gameplay Ability System）之间的中间层：
 *   - 持有 PC / PS / ASC / AS 四个核心 GAS 引用
 *   - 监听 GAS 属性变化（通过 ASC 的 GetGameplayAttributeValueChangeDelegate）
 *   - 将属性变化转化为蓝图可绑定的 Multicast Delegate 广播给 Widget
 *
 * 子类（如 UOverlayWidgetController）只需声明对应的 Delegate 并重写 BroadCastInitialValues()。
 */
UCLASS(Blueprintable, Abstract)
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()
public:
	/** 设置 GAS 所需的四个核心对象引用 */
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams);

	/**
	 * 广播初始属性值
	 *
	 * 在 InitOverlay 流程中，Widget 创建并绑定 Controller 后立即调用，
	 * 确保 Widget 首次显示时就展示正确的数据（而非默认值 0）。
	 * 子类必须重写以广播各自的初始属性。
	 */
	UFUNCTION(BlueprintCallable)
	virtual void BroadCastInitialValues();
	virtual void BindCallbacksToDependencies();
	
protected:
	/** 玩家控制器 */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AAuraPlayerController> AuraPlayerController;

	/** 玩家状态（网络复制） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AAuraPlayerState> AuraPlayerState;

	/** 技能系统组件 */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	/** 属性集（包含 Health, Mana, MaxHealth, MaxMana 等） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;
};
