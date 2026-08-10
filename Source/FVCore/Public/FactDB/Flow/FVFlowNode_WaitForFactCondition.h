#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FactDB/FVFactTypes.h"
#include "FVFlowNode_WaitForFactCondition.generated.h"

/**
 * Waits until a fact condition group is satisfied.
 *
 * Unlike On Fact Changed, this checks the current state first and completes
 * immediately if the condition already holds, so it cannot miss a change that
 * happened before the node was reached.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Wait For Fact Condition"))
class FLICKERVOIDCORE_API UFVFlowNode_WaitForFactCondition : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_WaitForFactCondition();

protected:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void OnLoad_Implementation() override;
	virtual void Cleanup() override;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetNodeDescription() const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Fact")
	FFVFactConditionGroup Conditions;

private:
	void StartWaiting();
	void StopWaiting();
	void HandleWatchedFactChanged(int32 NewValue);

	bool bIsWaiting = false;
};
