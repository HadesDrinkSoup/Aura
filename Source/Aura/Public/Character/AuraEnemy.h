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
	/**
	 * 配置两件事：
	 *  1. 骨骼网格的 Visibility 通道设为 Block，让光标射线（ECC_Visibility）能命 中敌人；
	 *  2. 自建 ASC / AttributeSet / 血条组件（玩家角色是借用 PlayerState 的 ASC，这里相反）。
	 * ASC 用 Minimal 复制模式：敌人不需要把完整 GE 信息发给所有客户端，
	 * 只要让客户端看到表现（血量、Buff）即可，能显著降低带宽。
	 */
	AAuraEnemy();

	/** BeginPlay 中只做 ActorInfo 初始化；服务器与客户端都会进入，InitAbilityActorInfo 自身区分职责 */
	virtual void BeginPlay() override;
	
	/** 开启 CustomDepth 渲染，设置 Stencil 值为 CUSTOM_DEPTH_RED 以触发红色描边后处理 */
	virtual void HighlightActor() override;

	/** 关闭 CustomDepth 渲染，移除描边效果 */
	virtual void UnHighlightActor() override;
	
	/** 敌人等级直接取编辑器里配置的 Level（不依赖 PlayerState） */
	virtual int32 GetCharacterLevel() override;
	
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UPROPERTY(BlueprintReadOnly, Category= "Combat")
	bool bHitReact = false;
	
protected:
	/**
	 * 敌人侧的 GAS 初始化：ASC 与 AttributeSet 都在本对象上，
	 * 因此 OwnerActor 与 AvatarActor 都是 this；随后加载默认属性并创建血条。
	 * 没有幂等标志：只由 BeginPlay 调用一次。
	 */
	virtual void InitActorInfo() override;
	
	/** 创建并初始化敌人血条的 WidgetController（每个敌人生成一个独立实例） */
	virtual void InitHealthBar();
	
	/** 敌人等级：参与 GE 等级与 MMC 等级系数，不同敌人直接在 BP 里改 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Class Default ")
	int32 Level = 1;
	
	/** 基础移动速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Class Default ")
	float BaseWalkSpeed = 250.f;
	
	/** 头顶血条的 WidgetComponent；构造时挂到根组件上，由蓝图指定 WidgetClass 与显示参数 */
	UPROPERTY(EditDefaultsOnly, Category= "Class Default ")
	TObjectPtr<UWidgetComponent> HealthBar;
	
	/** 血条控制器类（TSubclassOf 是"类"不是实例，运行时需 NewObject 才能拿到对象） */
	UPROPERTY(EditDefaultsOnly, Category= "Class Default ")
	TSubclassOf<UEnemyHealthBarWidgetController> HealthBarControllerClass;
	
	/** 本敌人血条的 WidgetController 实例（缓存引用防止被 GC 回收） */
	UPROPERTY()
	TObjectPtr<UEnemyHealthBarWidgetController> HealthBarController;
};
