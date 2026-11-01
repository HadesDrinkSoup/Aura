// Study Game Ability System _Hades
//
// 敌人接口：为"可被光标选中的敌人"提供一个统一入口。
// AuraPlayerController::CursorTrace 每帧从光标位置投射一条射线，命中 Actor 后
// Cast<IEnemyInterface> 判断它是否是可交互的敌人，再调用高亮 / 取消高亮；
// 这样控制器无需知道对面到底是 AuraEnemy 还是别的什么类。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyInterface.generated.h"

UINTERFACE(MinimalAPI)
class UEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 敌人接口本体。
 * AAuraEnemy 通过 Custom Depth 实现高亮：SetRenderCustomDepth(true) 并写入模板值
 * CUSTOM_DEPTH_RED，由后处理材质把该模板值的像素描边，因此不需要替换角色材质。
 */
class AURA_API IEnemyInterface
{
	GENERATED_BODY()

public:
	/**
	 * 光标移动到该敌人身上时调用。
	 * 由 AuraPlayerController::CursorTrace 在"当前目标与上一帧不同"时触发一次，
	 * 而非每帧重复调用。
	 */
	virtual void HighlightActor() {}
	/**
	 * 光标离开该敌人时调用，与 HighlightActor 成对出现，负责撤销高亮状态；
	 * 若忘记撤销会出现"高亮残留"。
	 */
	virtual void UnHighlightActor() {}
};
