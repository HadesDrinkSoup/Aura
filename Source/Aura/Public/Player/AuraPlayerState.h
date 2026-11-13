// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "Interface/CombatInterface.h"
#include "AuraPlayerState.generated.h"

class UAuraAttributeSet;
class UAttributeSet;
class UAbilitySystemComponent;

/**
 * @brief Aura 项目的 PlayerState，实现 IAbilitySystemInterface 以支持 GAS
 * 
 * 职责：
 * - 作为 AbilitySystemComponent 和 AttributeSet 的持有者
 * - 在多人游戏中负责玩家状态（属性、技能）的网络同步
 * 
 * 实现 IAbilitySystemInterface 后，GAS 框架能通过该接口自动获取 ASC
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY() 
public:
	AAuraPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 获取 ASC
	virtual UAuraAbilitySystemComponent* GetAbilitySystemComponent() const override {return AbilitySystemComponent;}
	// 获取属性集的便捷访问方法
	UAuraAttributeSet* GetAttributeSet() const {return AttributeSet;}
	
	virtual int32 GetCharacterLevel() const override {return Level;}
	
protected:
	// 技能系统组件 —— GAS 核心，管理技能授予、激活与 GameplayEffect
	UPROPERTY(VisibleAnywhere, Category = "Aura|PlayerState|Components", meta = (DisplayName = "技能系统组件"))
	TObjectPtr<UAuraAbilitySystemComponent> AbilitySystemComponent;
	// 属性集 —— 存储玩家所有 GameplayAttribute（生命值、魔力值等）
	UPROPERTY(VisibleAnywhere, Category = "Aura|PlayerState|Components", meta = (DisplayName = "属性集"))
	TObjectPtr<UAuraAttributeSet> AttributeSet;
	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing= OnRep_Level, Category = "Aura|PlayerState|Defaults", meta = (DisplayName = "玩家等级"))
	int32 Level = 1;
	
	UFUNCTION()
	void OnRep_Level(const int32 OldLevel);
};