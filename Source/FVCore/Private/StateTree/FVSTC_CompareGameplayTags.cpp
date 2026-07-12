// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/FVSTC_CompareGameplayTags.h"

bool UFVSTC_CompareGameplayTags::TestCondition(FStateTreeExecutionContext& Context) const
{
	const bool bMatch = ExactMatch
		? InputTag.MatchesTagExact(ConditionTag)
		: InputTag.MatchesTag(ConditionTag);

	return Invert ? !bMatch : bMatch;
}

#if WITH_EDITOR
FText UFVSTC_CompareGameplayTags::GetDescription(
	const FGuid& ID, 
	FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, 
	EStateTreeNodeFormatting Formatting) const
{
	// Retrieve instance data for the current node (ourself)
	const UFVSTC_CompareGameplayTags* Instance = InstanceDataView.GetPtr<UFVSTC_CompareGameplayTags>();
	if (!Instance)
	{
		return NSLOCTEXT("FlickerVoid", "CompareGameplayTagsNoInstance", "Compare [Invalid Instance]");
	}

	FText MatchType = Instance->ExactMatch ? FText::FromString("==") : FText::FromString("Matches");
	FText InvertText = Instance->Invert ? FText::FromString(" (Inverted)") : FText::GetEmpty();
	FText InputTagText = FText::FromName(Instance->InputTag.GetTagName());
	FText ConditionTagText = FText::FromName(Instance->ConditionTag.GetTagName());

	return FText::Format(
		NSLOCTEXT("FlickerVoid", "CompareGameplayTagsDescription", "Compare {0} {1} {2}{3}"),
		InputTagText, MatchType, ConditionTagText, InvertText
	);
}
#endif