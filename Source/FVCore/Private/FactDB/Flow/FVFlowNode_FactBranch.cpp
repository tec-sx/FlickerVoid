#include "FactDB/Flow/FVFlowNode_FactBranch.h"

#include "AddOns/FlowNodeAddOn_PredicateAND.h"
#include "Interfaces/FlowPredicateInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_FactBranch)

namespace
{
	const FName TrueOutputPin(TEXT("True"));
	const FName FalseOutputPin(TEXT("False"));
}

UFVFlowNode_FactBranch::UFVFlowNode_FactBranch()
{
	InputPins = {FFlowPin(TEXT("In"))};
	OutputPins = {FFlowPin(TrueOutputPin), FFlowPin(FalseOutputPin)};

#if WITH_EDITOR
	Category = TEXT("Fact");
#endif
}

EFlowAddOnAcceptResult UFVFlowNode_FactBranch::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate,
	const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	// Only predicates make sense here; anything else would silently do nothing.
	if (IFlowPredicateInterface::ImplementsInterfaceSafe(AddOnTemplate))
	{
		return EFlowAddOnAcceptResult::TentativeAccept;
	}

	return EFlowAddOnAcceptResult::Reject;
}

void UFVFlowNode_FactBranch::ExecuteInput(const FName& PinName)
{
	const bool bResult = UFlowNodeAddOn_PredicateAND::EvaluatePredicateAND(AddOns);

	TriggerOutput(bResult ? TrueOutputPin : FalseOutputPin, true);
}

#if WITH_EDITOR
FString UFVFlowNode_FactBranch::GetNodeDescription() const
{
	int32 PredicateCount = 0;
	for (const UFlowNodeAddOn* AddOn : AddOns)
	{
		if (IFlowPredicateInterface::ImplementsInterfaceSafe(AddOn))
		{
			++PredicateCount;
		}
	}

	if (PredicateCount == 0)
	{
		return TEXT("No predicates: always True");
	}

	return FString::Printf(TEXT("All of %d predicate(s)"), PredicateCount);
}
#endif
