// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraWidgetRow.generated.h"

/**
 * 消息 Widget 数据表行
 *
 * 存放在 DataTable（如 DT_MessageWidgetDataTable）中，
 * 通过 GameplayTag（Message.*）查找，决定提示文本、图标与 Widget 类。
 *
 * 注意：
 *   1. 本结构体单独成头文件，避免 AuraWidgetController.h 与
 *      OverlayWidgetController.h 互相包含导致的 UHT 循环依赖。
 *   2. 引用本结构体的动态委托必须声明在结构体之后 —— UHT 按声明顺序
 *      生成代码，先声明委托会报 "code generation hash is zero"。
 */
USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 消息标签（Message.HealthPotion 等），查表键 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	/** 提示文本 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText MessageText = FText();

	/** 消息 Widget 类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAuraUserWidget> MessageWidgetClass = nullptr;

	/** 提示图标 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> Image = nullptr;
};

/** 消息行广播委托：Row — 查表得到的消息行数据（必须位于 FUIWidgetRow 之后） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowDelegate, FUIWidgetRow, Row);
