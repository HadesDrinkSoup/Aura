// Study Game Ability System _Hades

#include "AuraGameplayEffectContext.h"

/**
 * 上下文的自定义网络序列化。
 *
 * 布局（自底向上）：
 *   [基类 7 bit 的存在标记 + 按标记跟随的字段载荷]   ← FGameplayEffectContext::NetSerialize 负责
 *   [本类 3 bit：bit0 = bIsDodge，bit1 = bIsCriticalHit，bit2 = 有 DamageType] ← 本函数追加
 *   [DamageType 标签本体（仅 bit2 为 1 时）]                          ← 本函数追加
 *
 * 为什么先调基类、而不是像多数教程那样把基类的 7 个分支整段抄过来：
 *  - 抄写意味着一份引擎代码副本。引擎一旦增删字段（例如 UE5.8 基类里已经有
 *    bReplicateSourceObject / bReplicateInstigator / bReplicateEffectCauser 三个开关，
 *    抄旧版教程会漏掉），上下文就会在客户端解析错位——是最难查的一类 bug；
 *  - 先让基类完整写完再追加自己的 bit，顺序天然一致：读端也是先基类、后本类。
 * 代价是本类字段的 bit 必须追加在末尾，一旦发布就不能再插到基类前面。
 *
 * 位压缩策略：
 *  - 两个 bool 与基类「有位才有值」不同：直接把标记本身当成 bit 的内容（bit 存在即 true），
 *    所以无论命中结果如何都只多花 1 字节（3 bit 按 1 字节对齐写出）；
 *  - DamageType 是有载荷的字段，所以额外用 bit2 当「后面有没有标签」的存在标记；
 *  - 读端无条件用 bit 覆盖成员，避免上下文实例被复用时残留上一次的 true / 旧标签
 *    （基类的 bHasWorldOrigin 也是这么处理的）。
 *
 * 注意：以上只覆盖「老复制系统」（结构体属性 + NetSerialize）。若将来开启 Iris，
 * 引擎会改用 FGameplayEffectContextNetSerializer（见 GameplayEffectTypes.cpp 顶部警告），
 * 那里不认识这 3 bit 与标签，需要另外派生一个 FNetSerializer 才能继续传这些字段。
 */
bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// 基类把字段写进/读出同一条字节流：Instigator / EffectCauser / AbilityCDO /SourceObject / Actors / HitResult / WorldOrigin。
	// 保留它是引擎里 if (!Super::NetSerialize(...)) 的惯例，好处是引擎将来若改成可失败，这里自然跟随。
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);
	if (Ar.IsError())
	{
		bOutSuccess = false;
		return false;
	}

	// 位宽只需 1 字节：FArchive::SerializeBits 的实现是
	//   Serialize(V, (LengthBits + 7) / 8);
	//   if (IsLoading() && LengthBits % 8) ((uint8*)V)[LengthBits / 8] &= (1 << (LengthBits & 7)) - 1;
	// 即「一次搬运 ceil(N/8) 个字节 + 读端清掉最后一字节的高位」，
	// 不是逐 bit 打包。所以 3 bit 仍然只读写 1 字节：
	//  - 用 uint32 不会多花带宽，但会让人误以为有 4 字节，而且在大端机器上
	//    bit0/bit1 落在第 4 字节，只搬第 1 字节就会读错 —— 引擎基类用 uint8 也是这个原因；
	//  - 用 uint8 语义与传输量完全一致，读端 mask 一下即可。
	//
	// 三个 bit 的含义：bit0 = bIsDodge，bit1 = bIsCriticalHit，
	// bit2 = 「后面还跟了一个 DamageType 标签」。bit2 只是存在标记，标签本体用
	// FGameplayTag::NetSerialize 紧随其后写/读（它内部走 tag 的网络索引，比序列化 FName 更省）。
	uint8 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bIsDodge)
		{
			RepBits |= 1 << 0;
		}
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 1;
		}
		if (DamageType.IsValid())
		{
			RepBits |= 1 << 2;
		}
	}

	// 位宽必须与实际使用的 bit 数一起增长：这里若仍写 2，bit2 会被截断，
	// 客户端永远读不到元素标签（而且不会有任何报错，属于最难查的静默失败）。
	Ar.SerializeBits(&RepBits, 3);

	if (Ar.IsSaving())
	{
		if (RepBits & (1 << 2))
		{
			DamageType.NetSerialize(Ar, Map, bOutSuccess);
		}
	}
	else
	{
		bIsDodge = (RepBits & (1 << 0)) != 0;
		bIsCriticalHit = (RepBits & (1 << 1)) != 0;

		// 读端先清空：上下文实例可能被复用，不清会残留上一次的元素标签
		DamageType = FGameplayTag();
		if (RepBits & (1 << 2))
		{
			DamageType.NetSerialize(Ar, Map, bOutSuccess);
		}
	}

	if (Ar.IsError())
	{
		bOutSuccess = false;
		return false;
	}

	bOutSuccess = true;
	return true;
}
