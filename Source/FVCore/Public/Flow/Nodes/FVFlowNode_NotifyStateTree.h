#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_NotifyStateTree.generated.h"

UCLASS()
class FLICKERVOIDCORE_API UFVFlowNode_NotifyStateTree : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_NotifyStateTree();
	
	UPROPERTY(EditAnywhere, Category = "State Tree")
	FGameplayTag EventTag;

protected:
	virtual void ExecuteInput(const FName& PinName) override;
};
