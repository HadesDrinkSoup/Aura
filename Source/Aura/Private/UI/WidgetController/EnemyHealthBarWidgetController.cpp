// Study Game Ability System _Hades


#include "UI/WidgetController/EnemyHealthBarWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

void UEnemyHealthBarWidgetController::BroadCastInitialValues()
{
	if (!AuraAttributeSet) return;

	// 客户端的 Health / MaxHealth 是由服务器复制过来的，BeginPlay 时可能仍是默认值 0。
	// 此时若照常广播，Widget 会按 0/0（SafeDivide 得 0）或 0/上限 计算百分比，
	// 被血条蓝图当成"受过伤"而显示出来 —— 这就是客户端启动时部分敌人出现血条的原因。
	// 因此只有两个值都有效时才广播；否则等复制到位后由 TryBroadcastInitialValues() 补播。
	if (AuraAttributeSet->GetMaxHealth() <= 0.f || AuraAttributeSet->GetHealth() <= 0.f) return;

	UE_LOG(LogTemp, Log, TEXT("[HB] BroadCastInitialValues Health=%.1f MaxHealth=%.1f"),
		AuraAttributeSet->GetHealth(), AuraAttributeSet->GetMaxHealth());

	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth(), -1.f);
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth(), -1.f);

	bInitialValuesBroadcast = true;
}

void UEnemyHealthBarWidgetController::TryBroadcastInitialValues()
{
	if (bInitialValuesBroadcast || !AuraAttributeSet) return;

	if (AuraAttributeSet->GetMaxHealth() > 0.f && AuraAttributeSet->GetHealth() > 0.f)
	{
		BroadCastInitialValues();
	}
}

void UEnemyHealthBarWidgetController::BindCallbacksToDependencies()
{
	if (bCallbacksBound) return;
	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute())
	.AddLambda([this](const FOnAttributeChangeData& Data)
	{
		// 初始值尚未广播：说明客户端属性还在陆续复制中，这次变化的 OldValue 没有意义
		// （通常是 0→100 的首次同步），不能直接丢给 Widget，否则会让血条闪出来。
		// 等 Health 与 MaxHealth 都就绪后，统一补一次 OldValue = -1 的初始广播，
		// 使客户端与服务器端走完全相同的显示路径。
		if (!bInitialValuesBroadcast)
		{
			TryBroadcastInitialValues();
			return;
		}

		// 客户端首次复制落地的第二次变化：Health 与 MaxHealth 在同一个网络包到达时，
		// 第一个回调会补播初始值（OldValue=-1），第二个回调的 OldValue 就是 0。
		// 0 值变化不是"受到伤害"，若转发给 Widget，蓝图会按 OldValue != -1 走"变化分支"，
		// 把血条 SetVisibility(Visible) 并起 5 秒隐藏定时器 —— 表现为启动时血条闪出。
		if (Data.OldValue <= 0.f) return;
		
		OnHealthChanged.Broadcast(Data.NewValue, Data.OldValue);
	});
	AuraAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute())
	.AddLambda([this](const FOnAttributeChangeData& Data)
	{
		if (!bInitialValuesBroadcast)
		{
			TryBroadcastInitialValues();
			return;
		}

		if (Data.OldValue <= 0.f) return;
		
		OnMaxHealthChanged.Broadcast(Data.NewValue, Data.OldValue);
	});
	bCallbacksBound = true;
}
