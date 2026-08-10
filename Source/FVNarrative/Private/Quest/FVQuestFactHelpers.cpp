#include "Quest/FVQuestFactHelpers.h"

#include "FVNarrativeTags.h"
#include "GameplayTagsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuestFactHelpers)

namespace FVQuestFactAspects
{
	const FName Stage(TEXT("Stage"));
	const FName Outcome(TEXT("Outcome"));
	const FName Failed(TEXT("Failed"));
	const FName Objective(TEXT("Objective"));
	const FName Counter(TEXT("Counter"));
	const FName Reached(TEXT("Reached"));
	const FName Completed(TEXT("Completed"));
}

namespace
{
	// Quest and objective ids are authored data, so the resulting tags are runtime tags
	// rather than natively declared ones. ErrorIfNotFound is false to avoid ensure spam.
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

FGameplayTag UFVQuestFactHelpers::MakeQuestTag(FName QuestId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {QuestId});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestStageTag(FName QuestId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {QuestId, FVQuestFactAspects::Stage});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestOutcomeTag(FName QuestId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {QuestId, FVQuestFactAspects::Outcome});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestFailedTag(FName QuestId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {QuestId, FVQuestFactAspects::Failed});
}

FGameplayTag UFVQuestFactHelpers::MakeObjectiveTag(FName QuestId, FName ObjectiveId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {QuestId, FVQuestFactAspects::Objective, ObjectiveId});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestCounterTag(FName QuestId, FName CounterId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {QuestId, FVQuestFactAspects::Counter, CounterId});
}

FGameplayTag UFVQuestFactHelpers::MakeChapterTag(FName ChapterId, FName Aspect)
{
	if (Aspect.IsNone())
	{
		return MakeChildTag(FVNarrativeTags::Fact_Chapter, {ChapterId});
	}

	return MakeChildTag(FVNarrativeTags::Fact_Chapter, {ChapterId, Aspect});
}

int32 UFVQuestFactHelpers::QuestStageToValue(EFVQuestStage Stage)
{
	return static_cast<int32>(Stage);
}
