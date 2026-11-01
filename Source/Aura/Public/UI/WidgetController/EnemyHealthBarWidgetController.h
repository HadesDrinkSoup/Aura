// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "EnemyHealthBarWidgetController.generated.h"

/**
 * 敌人血条 WidgetController
 *
 * 只服务单个敌人：数据源是敌人自身的 ASC / AttributeSet，
 * 由 AAuraEnemy::InitHealthBar() 创建并绑定到 HealthBar WidgetComponent 上的 Widget。
 * 蓝图通过 OnHealthChanged / OnMaxHealthChanged 更新血条进度。
 */
UCLASS()
class AURA_API UEnemyHealthBarWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadCastInitialValues() override;
	
	virtual void BindCallbacksToDependencies() override;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeValueChangedDelegate OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeValueChangedDelegate OnMaxHealthChanged;
	
private:
	/**
	 * 在 Health 与 MaxHealth 都有效后补做一次初始值广播。
	 * 客户端属性由服务器复制，BeginPlay 时可能还没到达；复制也可能分两帧先后到达。
	 * 只有两个值都就绪才广播，避免用 0/0 或 0/上限 这种中间态刷新血条。
	 */
	void TryBroadcastInitialValues();

	bool bCallbacksBound = false;

	/** 是否已经向 Widget 广播过初始值（服务器在 InitHealthBar 时立即置位） */
	bool bInitialValuesBroadcast = false;
};
