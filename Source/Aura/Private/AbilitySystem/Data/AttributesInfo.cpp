// Study Game Ability System _Hades

#include "AbilitySystem/Data/AttributesInfo.h"

FAuraAttributeInfo* UAttributesInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound)
{
	for (FAuraAttributeInfo& Info : AttributeInfo)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return &Info;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("未能在[%s]属性中找到对应标签[%s]"), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	return nullptr;
}
