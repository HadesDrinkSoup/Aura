// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "FloatingTextWidgetComponent.generated.h"

UCLASS()
class AURA_API UFloatingTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	//设置漂浮文本内容（蓝图可调用，蓝图子类也可重写）
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetFloatingText(const FText &Text, const FLinearColor& Color, const float FontSize);
};
