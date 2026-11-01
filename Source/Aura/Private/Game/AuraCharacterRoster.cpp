// Study Game Ability System _Hades
//
// @file  AuraCharacterRoster.cpp
// @brief 可选角色名单的实现：把软引用列表按需加载成对象数组。
//
// 名单里的条目全部是 TSoftObjectPtr，编辑器里配置时不会把角色资产拖进内存；
// 只有在真正需要展示（打开选人界面）时才同步加载。

#include "Game/AuraCharacterRoster.h"

#include "Game/PlayerCharacterDefinition.h"

/**
 * 按下标取一个角色定义（同步加载）。
 * 供蓝图使用（BlueprintPure），因此越界与加载失败都返回 nullptr 而不是断言。
 * 注意：这是同步加载，只适合在界面打开/交互时调用，不要放进 Tick。
 */
UPlayerCharacterDefinition* UAuraCharacterRoster::GetCharacter(int32 Index) const
{
	if (!Characters.IsValidIndex(Index))
	{
		return nullptr;
	}
	return Characters[Index].LoadSynchronous();
}

/**
 * 同步加载全部角色并按 SortOrder 升序返回（选人界面的主入口）。
 * 先 Reserve 再 Add，避免数组反复扩容；加载失败的条目直接跳过，不中断遍历。
 * SortOrder 相同时再按资产名排序，保证每次打开界面顺序稳定
 * （TMap / 资产浏览器顺序不保证稳定，不能作为展示顺序依据）。
 */
TArray<UPlayerCharacterDefinition*> UAuraCharacterRoster::GetAllCharacters() const
{
	TArray<UPlayerCharacterDefinition*> Result;
	Result.Reserve(Characters.Num());

	for (const TSoftObjectPtr<UPlayerCharacterDefinition>& Entry : Characters)
	{
		if (UPlayerCharacterDefinition* Character = Entry.LoadSynchronous())
		{
			Result.Add(Character);
		}
	}

	// SortOrder 相同时按资产名排序，保证每次打开界面顺序一致
	Result.Sort([](const UPlayerCharacterDefinition& A, const UPlayerCharacterDefinition& B)
	{
		return A.SortOrder != B.SortOrder ? A.SortOrder < B.SortOrder : A.GetName() < B.GetName();
	});

	return Result;
}
