#include "Conditions/FVInteractionConditions.h"

#include "Components/FVInteractorComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionConditions)

#define LOCTEXT_NAMESPACE "FVInteractionConditions"

bool FFVCondition_InteractorTags::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVInteractorComponent* Interactor = Context.Instigator ? Context.Instigator->FindComponentByClass<UFVInteractorComponent>() : nullptr;
	if (!Interactor)
	{
		return RequiredTags.IsEmpty();
	}

	const FGameplayTagContainer& SourceTags = Interactor->GrantedTags;
	if (SourceTags.HasAny(BlockedTags))
	{
		return false;
	}

	if (RequiredTags.IsEmpty())
	{
		return true;
	}

	return bRequireAll ? SourceTags.HasAll(RequiredTags) : SourceTags.HasAny(RequiredTags);
}

FText FFVCondition_InteractorTags::GetDescription() const
{
	TArray<FText> Parts;
	if (!RequiredTags.IsEmpty())
	{
		Parts.Add(FText::Format(LOCTEXT("Required", "Requires {0} of {1}"),
			bRequireAll ? LOCTEXT("All", "all") : LOCTEXT("Any", "any"),
			FText::FromString(RequiredTags.ToStringSimple())));
	}
	if (!BlockedTags.IsEmpty())
	{
		Parts.Add(FText::Format(LOCTEXT("Blocked", "Blocked by {0}"), FText::FromString(BlockedTags.ToStringSimple())));
	}
	return FText::Join(LOCTEXT("Sep", ", "), Parts);
}

#undef LOCTEXT_NAMESPACE
