#include "Quest/FVQuestFactHelpers.h"

#include "FVNarrativeTags.h"
#include "GameplayTagsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuestFactHelpers)

namespace FVQuestFactAspects
{
	const FName Stage(TEXT("Stage"));
	const FName Outcome(TEXT("Outcome"));
	const FName Objective(TEXT("Objective"));
	const FName Counter(TEXT("Counter"));
	const FName Reached(TEXT("Reached"));
	const FName Completed(TEXT("Completed"));
}

namespace
{
	// Quest and objective ids are authored data, so the resulting tags are runtime tags
	// rather than natively declared ones. ErrorIfNotFound is false to avoid ensure spam.
	// Four inline parts covers the deepest tag: <ChapterId>.<QuestId>.Objective.<ObjectiveId>.
	FGameplayTag MakeChildTag(const FGameplayTag& ParentTag, const TArray<FName, TInlineAllocator<4>>& Parts)
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

FGameplayTag UFVQuestFactHelpers::MakeChapterQuestsTag(FName ChapterId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {ChapterId});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestTag(FName ChapterId, FName QuestId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {ChapterId, QuestId});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestStageTag(FName ChapterId, FName QuestId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {ChapterId, QuestId, FVQuestFactAspects::Stage});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestOutcomeTag(FName ChapterId, FName QuestId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {ChapterId, QuestId, FVQuestFactAspects::Outcome});
}

FGameplayTag UFVQuestFactHelpers::MakeObjectiveTag(FName ChapterId, FName QuestId, FName ObjectiveId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {ChapterId, QuestId, FVQuestFactAspects::Objective, ObjectiveId});
}

FGameplayTag UFVQuestFactHelpers::MakeQuestCounterTag(FName ChapterId, FName QuestId, FName CounterId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Quest, {ChapterId, QuestId, FVQuestFactAspects::Counter, CounterId});
}

FGameplayTag UFVQuestFactHelpers::MakeChapterTag(FName ChapterId, FName Aspect)
{
	if (Aspect.IsNone())
	{
		return MakeChildTag(FVNarrativeTags::Fact_Chapter, {ChapterId});
	}

	return MakeChildTag(FVNarrativeTags::Fact_Chapter, {ChapterId, Aspect});
}

FGameplayTag UFVQuestFactHelpers::MakeChapterStageTag(FName ChapterId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Chapter, {ChapterId, FVQuestFactAspects::Stage});
}

FGameplayTag UFVQuestFactHelpers::MakeChapterOutcomeTag(FName ChapterId)
{
	return MakeChildTag(FVNarrativeTags::Fact_Chapter, {ChapterId, FVQuestFactAspects::Outcome});
}

int32 UFVQuestFactHelpers::QuestStageToValue(EFVQuestStage Stage)
{
	return static_cast<int32>(Stage);
}
