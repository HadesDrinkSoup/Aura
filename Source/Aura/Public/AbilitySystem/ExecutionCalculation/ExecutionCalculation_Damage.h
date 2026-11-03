// Study Game Ability System _Hades

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interface/CombatInterface.h"
#include "ExecutionCalculation_Damage.generated.h"

/**
 * 伤害结算用到的纯数学函数（无状态、无副作用），直接以 inline 形式放在头文件里。
 *
 * 为什么写在头文件而不另开 .cpp：
 *  - 三个函数都只有几行，编译期就能内联，不值得为它们多维护一对 .h/.cpp；
 *  - 用 inline 保证多个 TU 包含后只有一个实体，不产生重复定义；
 *  - 它们只服务本类，不进全局工具库，避免把「伤害公式」扩散成谁都能改的公共 API。
 */
namespace AuraDamageMath
{
	/**
	 * 防御的「价值刻度」K：K = DefenseBase + DefensePerLevel × (AttackerLevel − 1)。
	 * K 即「让减伤达到 50% 所需的防御值」，必须跟着等级涨，
	 * 否则护甲按等级线性增长时，后期减伤会一路逼近 100%。
	 *
	 * 注意 K 与减伤是反向的：Mitigation = D / (D + K)，K 越大 → 减伤越弱。
	 * 所以这条等级刻度只能跟**施法者**等级走：
	 *  - 跟受击者等级走（旧实现）：受击者每升一级护甲只涨 +1.5~+4，K 却涨 +10，
	 *    K 涨得更快 → 减伤反而变低 → 敌人升级后更脆，升到 2 级反而比 1 级多吃 1%~6% 伤害；
	 *  - 跟施法者等级走（现在）：同级互打时 K 与旧实现完全一致（双方等级相同，刻度相同），
	 *    受击者等级只通过护甲表达——等级更高 → 护甲更厚而 K 不变 → 更抗打；
	 *    等级更低 → 护甲更薄而 K 不变 → 更脆。等级差的效果有了，方向也是对的。
	 * @return 恒 > 0 的 K，可直接作分母
	 */
	inline float ComputeDefenseK(const float DefenseBase, const float DefensePerLevel, const int32 AttackerLevel)
	{
		const int32 SafeLevel = FMath::Max(AttackerLevel, 1);
		return DefenseBase + DefensePerLevel * static_cast<float>(SafeLevel - 1);
	}

	/**
	 * 单段伤害的减伤：Mitigation = Defense / (Defense + K)，结果 = Raw × (1 − Mitigation)。
	 *
	 * 用除法而不是「减防御」，是为了让三个端点同时成立：防御 0 → 0%；
	 * 防御 = K → 50%；防御 → ∞ → 趋近 100% 但永不免疫。
	 *
	 * @param RawDamage      减伤前的原始伤害，<= 0 时直接返回 0
	 * @param Defense        护甲或魔抗，负值按 0 处理（不产生"负防御增伤"）
	 * @param DefenseK       ComputeDefenseK 的结果，<= 0 时按 1 处理（避免 0/0）
	 * @param MinDamageRatio 结果占原始伤害的下限比例 [0,1]
	 */
	inline float ApplyMitigation(const float RawDamage, const float Defense, const float DefenseK, const float MinDamageRatio)
	{
		if (RawDamage <= 0.f)
		{
			return 0.f;
		}

		const float SafeDefense = FMath::Max(Defense, 0.f);
		const float SafeK = FMath::Max(DefenseK, 1.f);

		const float Mitigation = SafeDefense / (SafeDefense + SafeK);
		const float MitigatedDamage = RawDamage * (1.f - Mitigation);

		return FMath::Max(MitigatedDamage, RawDamage * FMath::Clamp(MinDamageRatio, 0.f, 1.f));
	}

	/**
	 * 反解角色等级（施法者、受击者通用）：能 Cast 成 ICombatInterface 就取 GetCharacterLevel。
	 * 不是所有对象（投射物、道具）都实现该接口，取不到时返回 FallbackLevel。
	 *
	 * @param Candidate     待解析对象，可为 nullptr
	 * @param FallbackLevel 解析失败时的返回值，由调用方决定兜底语义（施法者侧是 GE 等级，受击者侧退回施法者等级）
	 * @return 等级；解析不到时返回 FallbackLevel，且保证不为负
	 */
	inline int32 ResolveCharacterLevel(const UObject* Candidate, const int32 FallbackLevel = 0)
	{
		if (const ICombatInterface* CombatInterface = Cast<ICombatInterface>(Candidate))
		{
			const int32 Level = CombatInterface->GetCharacterLevel();
			if (Level > 0)
			{
				return Level;
			}
		}
		return FMath::Max(FallbackLevel, 0);
	}
}

/**
 * 一次伤害结算需要的属性捕获定义集合：源方攻击力 + 目标方防御，共 4 个。
 *
 * 为什么捕获方式不一样：
 *  - 源属性 snapshot = true：伤害在「施法那一刻」定格，投射物还在飞行途中施法者吃了减益也不改这一发；
 *  - 目标属性 snapshot = false：命中那一瞬间取最新值，否则"飞行期间套上的护盾"不起作用。
 * 由 cpp 里的 static AuraDamageStatics 实例持有，避免每次结算都重建捕获列表。
 */
struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalAttack)
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicalAttack)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor)
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicResistance)
	
	AuraDamageStatics()
	{
		// 源属性：snapshot = true，伤害在施法那一刻定格，飞行途中 buff 过期不改数
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalAttack, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, MagicalAttack, Source, true);
		// 目标属性：snapshot = false，命中那一瞬间取最新值
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, MagicResistance, Target, false);
	}
};

static const AuraDamageStatics& DamageStatics();

/**
 * 伤害结算 ExecutionCalculation：挂在「伤害 GE」的 Execution 上，每次施加对每个目标执行一次。
 *
 * 公式（物理/魔法两段独立计算，可叠加成混合伤害）：
 *   Raw        = 技能基础伤害 + 施法者攻击力 × AttackScaling
 *   Mitigation = 防御 / (防御 + K)，K = DefenseBase + DefensePerLevel × (施法者等级 - 1)
 *   Final      = Raw × (1 - Mitigation)，且不低于 Raw × MinDamageRatio
 * 最终写入 IncomingDamage（Meta 属性），由 UAuraAttributeSet::PostGameplayEffectExecute 统一扣血/飘字/受击/死亡。
 *
 * 设计依据（为什么是这几条曲线，而不是简单的加减乘除）：
 *  - 攻击力用「加算 + 系数」而不是乘算：技能基础伤害决定技能之间的强弱关系，
 *    攻击力提供同一条成长曲线上的额外收益，两者互不干扰，方便单独调整；
 *  - 防御用「除法减伤」而不是「减法减伤」：永远不会出现负伤害、也不会出现"护甲一高就完全免疫"，
 *    且天然收益递减（防御越低，每点护甲的收益越高）；
 *  - K 跟随施法者等级：护甲/魔抗由 MMC 按等级线性增长，如果 K 固定，
 *    后期减伤会一路逼近 100%；而 K 若跟受击者等级走，受击者每级只涨 1.5~4 点护甲、K 却涨 10 点，
 *    减伤反而随等级下滑（升级更脆）。把等级刻度交给施法者、把等级强度交给护甲，
 *    同级互打的数值与旧实现逐字相同，等级差才真正生效；
 *  - 保留 MinDamageRatio 下限：避免高防御目标被压出 0.3 这种"数字在跳但血条不动"的观感。
 *
 * 本类只做「数值」，命中/闪避/暴击属于概率判定，另行在施加 GE 前处理（见文件头注释）。
 */
UCLASS()
class AURA_API UExecutionCalculation_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UExecutionCalculation_Damage();
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	/**
	 * 攻击力 → 伤害的转换系数。
	 * 0 = 技能基础伤害决定一切（纯技能驱动）；1 = 攻击力原值全额计入（纯属性驱动）。
	 * 默认 0.6：以元素法师 1 级（物理攻击约 16、技能基础伤害约 15）为例，攻击力贡献约 10 点，占比四成。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Scaling", meta = (ClampMin = "0.0"))
	float AttackScaling = 0.6f;

	/**
	 * 防御 K 的基数：受击者防御等于 K 时，正好减免 50%。
	 * 调大 = 防御贬值（更偏进攻），调小 = 防御更值钱（更偏持久战）。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Mitigation", meta = (ClampMin = "1.0"))
	float DefenseBase = 50.f;

	/**
	 * 防御 K 的每级增量：K 随**施法者**等级线性增长（见 AuraDamageMath::ComputeDefenseK 的说明），
	 * 用来抵消护甲/魔抗的等级增长，让"同等级互打"的减伤比例稳定在同一区间，而不是随版本/等级漂移。
	 * 调大 = 后期高级施法者对低级目标伤害更高；调小 = 高等级双方互打时减伤更厚。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Mitigation", meta = (ClampMin = "0.0"))
	float DefensePerLevel = 10.f;

	/**
	 * 减伤下限比例：减伤之后至少保留「原始伤害 × 该比例」。
	 * 0.1 = 任何情况下至少打掉 10% 的原始伤害；0 = 允许减伤把伤害压到接近 0。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Mitigation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDamageRatio = 0.1f;
};
