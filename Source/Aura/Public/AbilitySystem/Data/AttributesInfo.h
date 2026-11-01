// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AttributesInfo.generated.h"

/**
 * 一条"属性展示信息"：把 GameplayTag 形式的属性标识，翻译成 UI 能直接显示的文案与当前数值。
 *
 * 用途：属性菜单（AttributesMenuController）遍历本结构体列表逐个取数、逐个刷新文本，
 * 让 UI 层完全不认识 AuraAttributeSet 的具体字段，只按 Tag 查数据。
 *
 * 数据来源：
 *  - AttributeTag / AttributeName / AttributeDescription 三个字段是**编辑期配置**（DataAsset 里填），
 *    在 UAttributesInfo 这个 DataAsset 的 AttributeInfo 数组中一项一项录入；
 *  - AttributeValue 是**运行期缓存**（没有 Edit 标记，编辑器里看不到也改不了），
 *    由 AttributesMenuController 每次属性变化时用 AttributeSet 的数值回填。
 */
USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
	GENERATED_BODY()
	/**
	 * 属性的来源标签，形如 Attributes.Primary.Strength、Attributes.Vital.Health、Attributes.Resistance.Fire。
	 * 这是整个系统的"属性身份"：MMC/GE 里按它定位属性，UI 里靠它从 AttributeSet 取值。
	 * 匹配时用 MatchesTagExact（精确匹配，不含父标签），因此必须写全路径，
	 * 只写 Attributes.Primary 不会命中任何一条记录。
	 * 默认值 FGameplayTag()（空标签）：未配置的项会被 FindAttributeInfoForTag 当作"找不到"处理。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();
	
	/**
	 * 显示给玩家的属性名称（如"力量"），纯文案，不参与任何逻辑判断。
	 * FText 而非 FString：支持本地化（本地化收集器会提取本字符串）。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();
	
	/**
	 * 显示给玩家的属性说明（tooltip 用），同样只是文案、支持本地化。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();
	
	/**
	 * 该属性的当前数值缓存，由 AttributesMenuController 在初始化时/属性变化回调里写入
	 * （AttributesMenuController.cpp:64：AttributeDelegate->Execute().GetNumericValue(AuraAttributeSet)）。
	 * 这里故意只标 BlueprintReadOnly、不标 Edit：它是跑起来才有的数据，
	 * 在 DataAsset 里手填没有意义，而且会被第一次刷新覆盖。
	 * 初值 0.f 只用于"还没刷新过"的瞬间，UI 拿到 0 说明初始化流程没跑到。
	 */
	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.f;
};

/**
 * 属性信息表 DataAsset：一份"属性标签 → 展示信息"的登记册，供属性菜单 UI 使用。
 *
 * 为什么要有它：AuraAttributeSet 里属性是 C++ 的 FGameplayAttribute 成员，
 * 蓝图里不方便按标签遍历/取显示名；把 Tag、名字、说明集中登记在一个资产里，
 * UI 就能用一条通用流程（遍历数组 → 按 Tag 取值 → 填文本）刷完所有属性，
 * 新增属性只要在资产里加一行，不用改 UI 蓝图。
 *
 * 配置方式（BP）：在 Bp_AttributesMenuController 的 "Attributes Information" 属性上指定本资产的实例，
 * 该属性为空时 AttributesMenuController.cpp:16 会打错误日志。
 */
UCLASS()
class AURA_API UAttributesInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	/**
	 * 按属性标签查找对应的展示信息；AttributeInfo 中查不到时返回 nullptr。
	 *
	 * @param AttributeTag   要查的属性标签（需完整路径，精确匹配）。
	 * @param bLogNotFound   找不到时是否打 Error 日志。默认 true 用于快速暴露配置遗漏；
	 *                       批量刷新 UI 时调用方传 false（AttributesMenuController.cpp:50），
	 *                       避免"某个属性没配显示文案"就把日志刷满。
	 * @return 命中项的指针（指向数组内部元素），未命中返回 nullptr——调用方必须判空。
	 *
	 * 实现上是线性遍历 + MatchesTagExact，数组规模在几十条以内，没有做索引/映射缓存。
	 */
	FAuraAttributeInfo* FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = true);
	
	/**
	 * 全部属性的登记列表，顺序即 UI 显示顺序（想调整界面顺序就调数组顺序）。
	 * 每一项的 AttributeTag 必须唯一，否则只有数组里靠前的那条会被 Find 命中（线性遍历先到先得）。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraAttributeInfo> AttributeInfo;
};
