#pragma once

#include "CoreMinimal.h"
#include "FVFlowNode_FactBase.h"
#include "FVFlowNode_OnFactDefined.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "On Fact Defined"))
class FLICKERVOIDCORE_API UFVFlowNode_OnFactDefined : public UFVFlowNode_FactBase
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_OnFactDefined();
	
protected:
	virtual void ExecuteInput( const FName& PinName ) override;
	virtual void OnLoad_Implementation() override;

	virtual void StartObserving();
	virtual void StopObserving();

	void OnEventReceived( int32 CurrentValue );

	virtual void Cleanup() override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif	
};
