// Study Game Ability System _Hades
//
// 战斗接口：为"对象可以被战斗系统统一处理"提供一份契约。
// 凡参与战斗结算的对象（玩家角色、敌人等）都实现本接口，技能、MMC、UI 等模块
// 只需 Cast<ICombatInterface> 即可拿到等级与武器位置，而不必依赖 AuraCharacter /
// AuraEnemy 这些具体类型，从而避免上层代码与具体职业耦合。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

/**
 * ICombatInterface 配套的 UObject 空壳。
 * UINTERFACE 必须与一个 UObject 子类配对（类名去掉 I 前缀），它只承担反射注册，
 * 不包含任何逻辑；真正的函数声明写在下面的 ICombatInterface 中。
 *  - MinimalAPI：反射信息仅在本模块内使用，减少导出符号；
 *  - BlueprintType：允许蓝图把实现了本接口的对象当作接口类型来使用（Cast 到接口）。
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * 战斗接口本体，由具体角色类重写：
 *  - AAuraCharacterBase 实现 GetCombatSocketLocation（武器插槽位置）；
 *  - AAuraCharacter / AAuraEnemy 分别实现 GetCharacterLevel（等级来源不同）。
 * 注意下面的函数都带有默认实现（返回 0 / 零向量），实现者忘记重写也不会崩溃，
 * 但调用方会拿到无效数值，所以把"默认值"当作"该对象不参与此项战斗计算"来理解。
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

public:
	/**
	 * 角色等级（供 MMC 与 GameplayEffectSpec 使用）。
	 * 调用方：UAuraMMCBase::GetCharacterLevelFromSpec 在计算幅值时按等级参与公式；
	 * 技能层在构建 Spec 时用它设置 EffectLevel。
	 * 返回 0 表示该对象没有等级概念，调用方应自行兜底（例如 FMath::Max(Level, 1)）。
	 */
	virtual int32 GetCharacterLevel() {return 0;};
	/**
	 * 武器尖端的世界坐标，作为近战判定、投射物生成点与特效原点的统一基准。
	 * 调用方：UAuraProjectileSpell 发射投射物时读取该坐标决定出生位置与朝向。
	 * 默认返回零向量，表示该对象没有可用的战斗插槽。
	 */
	virtual FVector GetCombatSocketLocation() {return FVector(0,0,0);};
	
	/**
	 * 让角色转向指定目标点；仅在蓝图（BlueprintImplementableEvent）中实现，
	 * 用于敌人在攻击前把朝向对准玩家，保证动画与特效的朝向正确。
	 * C++ 侧暂无调用点，由蓝图逻辑在需要时驱动。
	 * @param TargetLocation 需要朝向的世界坐标
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& TargetLocation);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();
};
