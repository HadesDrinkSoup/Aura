// Study Game Ability System _Hades

/**
 * @file AuraEnemy.h
 * @brief 敌方角色 —— 继承 AAuraCharacterBase，实现 IEnemyInterface 光标高亮接口。
 *
 * 高亮机制基于 Custom Depth + Stencil：
 * - HighlightActor 开启 CustomDepth 并写入 CUSTOM_DEPTH_RED (250) 模板值
 * - UnHighlightActor 关闭 CustomDepth
 * - 后处理材质读取 CustomStencil 缓冲区绘制描边，实现视觉高亮
 */

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interface/EnemyInterface.h"
#include "AuraEnemy.generated.h"

class UEnemyHealthBarWidgetController;
class UWidgetComponent;
/**
 * 敌方角色 —— 可被玩家光标检测并高亮
 *
 * 继承链：ACharacter → AAuraCharacterBase（带 Weapon 骨骼网格组件）→ AAuraEnemy
 * 接口：IEnemyInterface，由 AuraPlayerController::CursorTrace() 驱动高亮切换
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
public:
	AAuraEnemy();
	
	virtual void BeginPlay() override;
	
	/** 开启 CustomDepth 渲染，设置 Stencil 值为 CUSTOM_DEPTH_RED 以触发红色描边后处理 */
	virtual void HighlightActor() override;

	/** 关闭 CustomDepth 渲染，移除描边效果 */
	virtual void UnHighlightActor() override;
	
	virtual int32 GetCharacterLevel() override;
	
protected:
	virtual void InitActorInfo() override;
	
	/** 创建并初始化敌人血条的 WidgetController（每个敌人生成一个独立实例） */
	virtual void InitHealthBar();
	
	/** 敌人等级：参与 GE 等级与 MMC 等级系数，不同敌人直接在 BP 里改 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Class Default ")
	int32 Level = 1;
	
	UPROPERTY(EditDefaultsOnly, Category= "Class Default ")
	TObjectPtr<UWidgetComponent> HealthBar;
	
	UPROPERTY(EditDefaultsOnly, Category= "Class Default ")
	TSubclassOf<UEnemyHealthBarWidgetController> HealthBarControllerClass;
	
	/** 本敌人血条的 WidgetController 实例（缓存引用防止被 GC 回收） */
	UPROPERTY()
	TObjectPtr<UEnemyHealthBarWidgetController> HealthBarController;
};
