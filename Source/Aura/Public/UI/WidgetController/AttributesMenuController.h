// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AttributesMenuController.generated.h"

class UAttributesInfo;
struct FAuraAttributeInfo;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeInfoChangedDelegate, FAuraAttributeInfo, AttributeInfo);

UCLASS()
class AURA_API UAttributesMenuController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadCastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeInfoChangedDelegate AttributeInfoChangedDelegate;

protected:
	/** 防止 BindCallbacksToDependencies 重复绑定 */
	bool bCallbacksBound = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Attributes Information")
	TObjectPtr<UAttributesInfo> AttributeInfo;
};
