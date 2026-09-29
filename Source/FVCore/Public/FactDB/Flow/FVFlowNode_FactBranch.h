#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_FactBranch.generated.h"

/**
 * Branches on fact conditions supplied by child predicate AddOns.
 *
 * Attach one or more Fact Predicate AddOns to this node. They are combined with AND;
 * use a Predicate OR/composite AddOn as a child to express other logic. With no
 * predicates attached the node always takes True.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Fact Branch"))
class FLICKERVOIDCORE_API UFVFlowNode_FactBranch : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_FactBranch();

	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(
		const UFlowNodeAddOn* AddOnTemplate,
		const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;

protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif
};
