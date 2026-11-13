// Study Game Ability System _Hades

/**
 * @file AuraCharacterBase.cpp
 * @brief 角色基类实现：组件碰撞配置、初始属性 GE 的施加流程、默认技能的授予。
 *
 * 要点：
 *  - 初始属性只在服务器施加（GAS 的属性由服务器权威写入再复制给客户端）；
 *  - Primary → Derived → Affinity（灵属）→ 个体差异 → Pool 的顺序不可随意调换，否则会出现
 *    "满血上限变高但血条不满" 之类的取值问题；
 *  - 重复初始化（重生、重新 Possess）时必须先清掉旧的 Derived 实例，避免修正叠加。
 */

#include "Character/AuraCharacterBase.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "GameplayEffect.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

/** 构造：创建武器组件、绑定手部插槽，并配置胶囊体与网格体的碰撞响应 */
AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick  = false;
	
	// 武器是纯表现 + 打击点载体：挂在手部插槽上，不参与任何碰撞
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), "WeaponHandSocket");
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 胶囊体不产生 Overlap：角色间的接触交给网格体/其他逻辑处理，避免重复事件
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	// 相机通道忽略胶囊体，防止俯视角下相机贴到角色身上
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// 光标射线（GetHitResultUnderCursor(ECC_Visibility)）只能命中对 Visibility 通道 Block 的物体，
	// 而引擎默认的 Pawn / CharacterMesh 碰撞配置都把 Visibility 设为 Ignore
	// （见 Config/DefaultEngine.ini:115 的 Pawn、:117 的 CharacterMesh），
	// 结果就是所有角色对光标射线都是"透明"的。敌人此前在 AuraEnemy 构造函数里单独打开了
	// 网格体的 Visibility 响应，玩家角色却没有：瞄准另一个玩家时射线会直接穿过对方身体打中地面，
	// 落点自然变成他脚下的地板。
	// 这里在基类统一打开：胶囊体保证"点到即选中"（简单碰撞始终存在，不依赖网格体的 PhysicsAsset），
	// 网格体保证身体边缘、四肢等超出胶囊体的部分也能命中。
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	// 网格体承担射线检测（敌人高亮用 ECC_Visibility）与投射物命中（ECC_Projectile Overlap）
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
}

/** 返回 GAS 使用的 ASC；玩家侧来自 PlayerState，敌人侧自建，未初始化时返回 nullptr */
UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

/** 取武器骨骼网格上插槽的世界坐标，作为近战判定/特效生成的原点 */
FVector AAuraCharacterBase::GetCombatSocketLocation() const
{
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAuraCharacterBase::Death_Implementation()
{
	// 死亡只允许由服务器发起：客户端不能自己去调 Death，只能通过 MulticastHandleDeath 收到广播
	if (!HasAuthority()) return;
	
	// 注意：这里必须调用 RPC 本身 MulticastHandleDeath()，
	// 直接调 MulticastHandleDeath_Implementation() 会跳过 UFunction 的 RPC 入口，
	// 结果是只有服务器执行，客户端永远收不到死亡通知（不会倒、不会脱武器、不会关碰撞）。
	MulticastHandleDeath();
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld,true));
	Weapon->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	Dissolve();
}

/**
 * 从全局职业配置表（DA_CharacterClassInfo）取本角色职业的默认配置。
 * 通过 UAuraAbilitySystemLibrary::GetCharacterClassInfo(this) 读取 GameMode 上的表；
 * 表或职业项缺失时返回 nullptr，调用方需按"无职业配置"降级处理。
 */
const FCharacterClassDefaultInfo* AAuraCharacterBase::FindClassDefaultInfo() const
{
	const UCharacterClassInfo* ClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(this);
	return ClassInfo ? ClassInfo->FindCharacterClassInformation(CharacterClass) : nullptr;
}

/**
 * 合并"本 BP 附加技能 + 职业默认技能 + 表里的通用能力"。
 * 顺序固定为：本 BP 的 StartupAbilities → 职业的 StartupAbilities → CommonAbilities，
 * 全部用 AddUnique 追加，因此三者是**并集**而不是覆盖关系：同一个技能类无论配在几处，
 * 最终只会被授予一次（先出现的保留在前，顺序只影响 Spec 列表顺序，不影响能否激活）。
 *
 * 通用能力放在最后：它表达的是"所有角色都会的东西"（受击反应、闪避等），
 * 与具体职业无关，所以即使 CharacterClass 在表里查不到，也仍然会下发。
 */
TArray<TSubclassOf<UAuraGameplayAbility>> AAuraCharacterBase::GetCombinedStartupAbilities() const
{
	TArray<TSubclassOf<UAuraGameplayAbility>> Abilities = StartupAbilities;

	// 表只取一次：职业默认信息与通用能力都挂在同一份 DA_CharacterClassInfo 上，
	// 合并成一次查询既省一次软引用解析，也让两段逻辑共享同一个"表是否存在"的判空。
	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(this);
	if (!CharacterClassInfo)
	{
		// 未配置表：只下发本 BP 自己配的技能，不崩溃（与属性下发链路的降级方式一致）
		return Abilities;
	}

	if (const FCharacterClassDefaultInfo* ClassDefaults = CharacterClassInfo->FindCharacterClassInformation(CharacterClass))
	{
		for (const TSubclassOf<UAuraGameplayAbility>& Ability : ClassDefaults->StartupAbilities)
		{
			Abilities.AddUnique(Ability);
		}
	}

	// 通用能力：全职业共享的起始技能，例如敌人与玩家都要有的受击反应。
	// 跳过空槽位：BP 里的 TArray<TSubclassOf<>> 很容易留下空元素，
	// 而 AddCharacterAbilities 内部对每个元素做 CastChecked，空技能类会走到断言上。
	for (const TSubclassOf<UAuraGameplayAbility>& Ability : CharacterClassInfo->CommonAbilities)
	{
		if (Ability)
		{
			Abilities.AddUnique(Ability);
		}
	}

	return Abilities;
}

void AAuraCharacterBase::Dissolve()
{
	if (WeaponDissolveMaterialInstance)
	{
		UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMaterialInstance);
		StartWeaponDissolveTimeLine(DynamicMaterialInstance);
	}
	if (MeshDissolveMaterialInstance)
	{
		UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MeshDissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMaterialInstance);
		StartMeshDissolveTimeLine(DynamicMaterialInstance);
	}
}

/**
 * 施加角色的初始属性 GE（仅在服务器执行）。
 * 解析优先级：本 BP 显式配置的 GE（个体覆盖） > 职业配置表中的默认 GE。
 * 施加顺序及原因：
 *   1) Primary   ：写入力量/智力/敏捷/活力等主属性；
 *   2) Derived   ：由主属性经 MMC 算出派生属性，必须在 Primary 之后；
 *   3) Affinity  ：灵属（元素亲和力）通用 GE（表上的 CommonElementAffinity，五项各 +10），
 *                  再加 BP 上的 AdditionalElementAffinityEffects 个体差异；
 *   4) Additional：个体差异（精英加成、装备、诅咒），放在 Pool 之前，
 *                  这样池属性 GE 能读到最终的 MaxHealth/MaxMana；
 *   5) Pool      ：最后按最终上限把 Health/Mana 填满。
 */
void AAuraCharacterBase::InitDefaultAttributes()
{
	if (!HasAuthority()) return;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 解析顺序：本 BP 显式配置的 GE（个体覆盖） > 职业配置表中的默认 GE
	const FCharacterClassDefaultInfo* ClassDefaults = FindClassDefaultInfo();
	const TSubclassOf<UGameplayEffect> PrimaryAttributeEffect =
		PrimaryAttributes ? PrimaryAttributes : (ClassDefaults ? ClassDefaults->PrimaryAttributes : nullptr);
	const TSubclassOf<UGameplayEffect> DerivedAttributeEffect =
		DerivedAttributes ? DerivedAttributes : (ClassDefaults ? ClassDefaults->DerivedAttributes : nullptr);
	const TSubclassOf<UGameplayEffect> PoolAttributeEffect =
		PoolAttributes ? PoolAttributes : (ClassDefaults ? ClassDefaults->PoolAttributes : nullptr);

	// 灵属通用 GE 挂在表本身（所有职业共用），不随职业变化
	const UCharacterClassInfo* ClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(this);
	const TSubclassOf<UGameplayEffect> CommonElementAffinity = ClassInfo ? ClassInfo->CommonElementAffinity : nullptr;

	// 各类 GE 一个都没配：没有任何属性可施加，直接返回（避免白建上下文）
	if (!PrimaryAttributeEffect && !DerivedAttributeEffect && !CommonElementAffinity
		 && !PoolAttributeEffect && !AdditionalEffect)
	{
		return;
	}

	// 上下文只构建一次，所有 GE 共享，保证 Instigator/SourceObject 一致（MMC 与日志都依赖它）
	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	if (EffectContextHandle.IsValid() && EffectContextHandle.Get())
	{
		if (AController* OwnerController = GetController())
		{
			EffectContextHandle.Get()->AddInstigator(OwnerController, this);
		}
		EffectContextHandle.Get()->AddSourceObject(this);
	}

	// GE 等级 = 角色等级：主要属性 GE 用 CurveTable 时可以直接按等级取数值
	const int32 EffectLevel = FMath::Max(GetCharacterLevel(), 1);

	// Apply Primary attributes (if present)
	if (PrimaryAttributeEffect)
	{
		ApplyEffectToSelf(PrimaryAttributeEffect, ASC, EffectContextHandle, EffectLevel);
	}
	
	if (DerivedAttributeEffect)
	{
		// 派生属性 GE 是 Infinite（主属性变化时由 GAS 自动重算），
		// 重复初始化（例如重生）时先移除旧实例，否则修正会叠加。
		FGameplayEffectQuery Query;
		Query.EffectDefinition = DerivedAttributeEffect;
		for (const FActiveGameplayEffectHandle& Handle : ASC->GetActiveEffects(Query))
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}

		ApplyEffectToSelf(DerivedAttributeEffect, ASC, EffectContextHandle, EffectLevel);
	}

	// 灵属通用基础值：表上的 CommonElementAffinity（Instant，五项各 Add 10），与职业无关。
	// 未配置则跳过，灵属保持 0；持续型加成请放 AdditionalElementAffinityEffects。
	if (CommonElementAffinity)
	{
		ApplyEffectToSelf(CommonElementAffinity, ASC, EffectContextHandle, EffectLevel);
	}
	
	// 个体差异（精英加成、装备、诅咒等）：在 Derived / 灵属之后、Pool 之前施加。。
	if (AdditionalEffect)
	{
		ApplyEffectToSelf(AdditionalEffect, ASC, EffectContextHandle, EffectLevel);
	}

	// Apply Pool attributes (if present)：最后按最终的 MaxHealth/MaxMana 填满池属性
	if (PoolAttributeEffect)
	{
		ApplyEffectToSelf(PoolAttributeEffect, ASC, EffectContextHandle, EffectLevel);
	}
}

/**
 * 为自身创建并施加一个 GE。
 * 失败分支：ASC 或 GE 类为空、Spec 句柄无效时静默返回——
 * 初始属性属于"尽力而为"的配置，缺一个 GE 不应让角色初始化流程崩溃。
 */
void AAuraCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> EffectClass, UAbilitySystemComponent* ASC, const FGameplayEffectContextHandle& EffectContext, const float Level)
{
	if (!ASC || !EffectClass) return;

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, Level, EffectContext);
	if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

/**
 * 授予默认技能。
 * 技能是同步到客户端的复制数据，只在服务器执行；ASC 尚未就绪时直接跳过，
 * 由子类在下次初始化时再调用。
 */
void AAuraCharacterBase::AddCharacterAbilities() const
{
	if (!HasAuthority() || !AbilitySystemComponent) return;
	AbilitySystemComponent->AddCharacterAbilities(GetCombinedStartupAbilities());
}
