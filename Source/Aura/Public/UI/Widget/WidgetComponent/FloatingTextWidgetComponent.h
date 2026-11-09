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
	/**
	 * 设置漂浮文本内容 —— 纯蓝图事件，C++ 只负责把数据递过来。
	 * 文案（伤害数字 /「未命中」）、颜色、字号、动画、销毁时机全部在蓝图子类里实现。
	 * @param InDamage       最终伤害数值（暴击倍率已乘好；闪避时为 0）
	 * @param bIsDodge       本次攻击是否被闪避
	 * @param bIsCriticalHit 本次攻击是否暴击
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetFloatingText(float InDamage, bool bIsDodge, bool bIsCriticalHit);
};
