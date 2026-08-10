#include "FactDB/Flow/FVFlowNode_FactBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_FactBase)

UE_DEFINE_GAMEPLAY_TAG( FlowNodeStyle::Fact, "Flow.NodeStyle.Node.Fact" );

UFVFlowNode_FactBase::UFVFlowNode_FactBase()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::Fact;
	Category = TEXT("Fact");
#endif
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
