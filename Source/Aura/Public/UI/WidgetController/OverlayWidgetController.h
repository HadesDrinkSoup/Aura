// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/Widget/AuraWidgetRow.h"
#include "OverlayWidgetController.generated.h"

UCLASS(Blueprintable)
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	/** 重写基类：广播 Health 和 MaxHealth 的初始值 */
	virtual void BroadCastInitialValues() override;
	
	virtual void BindCallbacksToDependencies() override;

	/** 生命值变化时广播的委托（蓝图绑定到 HealthGlobe 更新 UI） */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeValueChangedDelegate OnHealthChanged;

	/** 最大生命值变化时广播的委托（蓝图绑定到 HealthGlobe 更新进度条上限） */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeValueChangedDelegate OnMaxHealthChanged;

	/** 魔法值变化时广播的委托 */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeValueChangedDelegate OnManaChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeValueChangedDelegate OnMaxManaChanged;
	
	UPROPERTY(BlueprintAssignable, Category="GAS|Message")
	FMessageWidgetRowDelegate MessageWidgetRow;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MessageWidgetData")
	UDataTable* MessageWidgetDataTable;
	
	template <typename  T>
	static T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);
	
private:
	/** 防止 BindCallbacksToDependencies 重复绑定 */
	bool bCallbacksBound = false;
};

template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
}
