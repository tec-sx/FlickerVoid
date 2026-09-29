#include "FactDB/Flow/FVFlowNode_FactBase.h"

#include "AddOns/FlowNodeAddOn_PredicateAND.h"
#include "FactDB/Flow/FVFlowNodeAddOn_SetFact.h"
#include "Interfaces/FlowPredicateInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_FactBase)

UE_DEFINE_GAMEPLAY_TAG( FlowNodeStyle::Fact, "Flow.NodeStyle.Node.Fact" );

UFVFlowNode_FactBase::UFVFlowNode_FactBase()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Fact;
	Category = TEXT("Fact");
#endif
}

EFlowAddOnAcceptResult UFVFlowNode_FactBase::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate,
	const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	if (IFlowPredicateInterface::ImplementsInterfaceSafe(AddOnTemplate) || (IsValid(AddOnTemplate) && AddOnTemplate->IsA<UFVFlowNodeAddOn_SetFact>()))
	{
		return EFlowAddOnAcceptResult::TentativeAccept;
	}

	return Super::AcceptFlowNodeAddOnChild_Implementation(AddOnTemplate, AdditionalAddOnsToAssumeAreChildren);
}

bool UFVFlowNode_FactBase::ArePredicateAddOnsSatisfied() const
{
	return UFlowNodeAddOn_PredicateAND::EvaluatePredicateAND(AddOns);
}

#if WITH_EDITOR
EDataValidationResult UFVFlowNode_FactBase::ValidateNode()
{
	if (Fact.IsValid())
	{
		return EDataValidationResult::Valid;
	}

	ValidationLog.Error<UFlowNode>(TEXT("Missing Tag"), this);
	return EDataValidationResult::Invalid;
}
#endif
