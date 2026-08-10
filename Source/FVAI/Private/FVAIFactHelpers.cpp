#include "FVAIFactHelpers.h"

#include "FVAITags.h"
#include "GameplayTagsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAIFactHelpers)

namespace FVNpcFactAspects
{
	const FName Met(TEXT("Met"));
	const FName Alive(TEXT("Alive"));
	const FName Trust(TEXT("Trust"));
	const FName Disposition(TEXT("Disposition"));
	const FName Knows(TEXT("Knows"));
	const FName Reputation(TEXT("Reputation"));
}

namespace
{
	// NPC and faction ids come from authored data, so these resolve to runtime tags.
	FGameplayTag MakeChildTag(const FGameplayTag& ParentTag, const TArray<FName, TInlineAllocator<3>>& Parts)
	{
		if (!ParentTag.IsValid())
		{
			return FGameplayTag();
		}

		FString TagString = ParentTag.ToString();
		for (const FName& Part : Parts)
		{
			if (Part.IsNone())
			{
				return FGameplayTag();
			}

			TagString.AppendChar(TEXT('.'));
			Part.AppendString(TagString);
		}

		return UGameplayTagsManager::Get().RequestGameplayTag(FName(*TagString), /*ErrorIfNotFound*/ false);
	}
}

FGameplayTag UFVAIFactHelpers::MakeNpcTag(FName NpcId)
{
	return MakeChildTag(FVAITags::Fact_NPC, {NpcId});
}

FGameplayTag UFVAIFactHelpers::MakeNpcFactTag(FName NpcId, FName Aspect)
{
	return MakeChildTag(FVAITags::Fact_NPC, {NpcId, Aspect});
}

FGameplayTag UFVAIFactHelpers::MakeNpcKnowsTag(FName NpcId, FName TopicId)
{
	return MakeChildTag(FVAITags::Fact_NPC, {NpcId, FVNpcFactAspects::Knows, TopicId});
}

FGameplayTag UFVAIFactHelpers::MakeFactionFactTag(FName FactionId, FName Aspect)
{
	if (Aspect.IsNone())
	{
		return MakeChildTag(FVAITags::Fact_Faction, {FactionId});
	}

	return MakeChildTag(FVAITags::Fact_Faction, {FactionId, Aspect});
}
