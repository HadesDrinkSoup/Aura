// Study Game Ability System _Hades
// 输入配置：把「输入动作」与「GameplayTag」绑定起来，
// 供 Aura 通过 GameplayTag 查询并触发对应的能力（Ability）。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AuraInputConfig.generated.h"

class UInputAction;

/**
 * 一条「输入动作 ↔ GameplayTag」的映射记录。
 * 在数据资产（DataAsset）中配置多组映射，运行时即可
 * 通过 GameplayTag 反查出对应的输入动作。
 */
USTRUCT(BlueprintType)
struct FAuraInputAction
{
	GENERATED_BODY()
	
	// 实际绑定的输入动作（例如 IA_Fire、IA_Jump 等）
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;
	
	// 该输入动作对应的 GameplayTag（用于标识触发的能力）
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

/**
 * 输入配置数据资产（UDataAsset）。
 * 存放所有「输入动作 ↔ GameplayTag」的映射，
 * 由角色/玩家控制器在运行时读取，用于把玩家输入路由到 GAS 能力。
 */
UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	/**
	 * 根据给定的 GameplayTag 查找对应的输入动作。
	 * @param InputTag  要查找的 GameplayTag
	 * @param bLogFound 为 true 时会在找到/未找到时打印日志
	 * @return 匹配的 UInputAction 指针；找不到时返回 nullptr
	 */
	const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag, bool bLogFound = false) const;
	
	// 所有可用的输入动作映射（在编辑器 DataAsset 中配置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraInputAction> AbilityInputActions;
};
