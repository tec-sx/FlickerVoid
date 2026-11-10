#include "Tags/FVTagConditions.h"

#include "Tags/FVTagLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVTagConditions)

#define LOCTEXT_NAMESPACE "FVTagConditions"

bool FFVCondition_ActorTags::EvaluateImpl(const FFVConditionContext& Context) const
{
	const AActor* Actor = Context.GetActor(bCheckTarget, StaticStruct());
	return UFVTagLibrary::ActorMatchesTags(Actor, RequiredTags, BlockedTags, bRequireAll);
}

FText FFVCondition_ActorTags::GetDescription() const
{
	TArray<FText> Parts;
	if (!RequiredTags.IsEmpty())
	{
		const FText Count = bRequireAll ? LOCTEXT("All", "all") : LOCTEXT("Any", "any");
		const FText Tags = FText::FromString(RequiredTags.ToStringSimple());
		Parts.Add(FText::Format(LOCTEXT("Required", "Requires {0} of {1}"), Count, Tags));
	}
	if (!BlockedTags.IsEmpty())
	{
		const FText Tags = FText::FromString(BlockedTags.ToStringSimple());
		Parts.Add(FText::Format(LOCTEXT("Blocked", "Blocked by {0}"), Tags));
	}
	return FText::Join(LOCTEXT("Sep", ", "), Parts);
}

#undef LOCTEXT_NAMESPACE
