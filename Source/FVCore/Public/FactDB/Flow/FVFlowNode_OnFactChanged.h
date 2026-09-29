#pragma once

#include "CoreMinimal.h"
#include "FactDB/Flow/FVFlowNode_FactBase.h"
#include "FVFlowNode_OnFactChanged.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "On Fact Changed"))
class FLICKERVOIDCORE_API UFVFlowNode_OnFactChanged : public UFVFlowNode_FactBase
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_OnFactChanged();
	
protected:

	UPROPERTY(EditAnywhere, Category = "Fact")
	bool bWaitForExactValue = true;
	
	UPROPERTY(EditAnywhere, Category = "Fact", meta = (EditCondition = bWaitForExactValue, EditConditionHides))
	int32 WantedValue;
	
	// This node will become Completed, if Success Limit > 0 and Success Count reaches this limit
	// Set this to zero, if you'd like receive events indefinitely (node would finish work only if explicitly Stopped)
	UPROPERTY(EditAnywhere, Category = "Lifetime", meta = (ClampMin = 0))
	int32 SuccessLimit;

	UPROPERTY(VisibleAnywhere, Category = "Lifetime", SaveGame)
	int32 SuccessCount;

	virtual void ExecuteInput( const FName& PinName ) override;
	virtual void OnLoad_Implementation() override;

	virtual void StartObserving();
	virtual void StopObserving();

	void OnEventReceived( int32 CurrentValue );

	virtual void Cleanup() override;

#if WITH_EDITOR
public:
	virtual FString GetNodeDescription() const override;
#endif
};
