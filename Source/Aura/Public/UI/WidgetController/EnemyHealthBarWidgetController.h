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
	bool bCallbacksBound = false;
};
