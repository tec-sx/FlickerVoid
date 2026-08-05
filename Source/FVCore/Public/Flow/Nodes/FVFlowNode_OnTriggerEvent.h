#pragma once

#include "Nodes/Actor/FlowNode_ComponentObserver.h"
#include "FVFlowNode_OnTriggerEvent.generated.h"

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable)
class FLICKERVOIDCORE_API UFVFlowNode_OnTriggerEvent : public UFlowNode_ComponentObserver
{
	GENERATED_BODY()
public:
	UFVFlowNode_OnTriggerEvent();
	
#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
#endif
	
protected:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	FGameplayTagContainer OverlappedActorTags;

	bool bReactOnOverlapping;

	virtual void ExecuteInput(const FName& PinName) override;
	
	virtual void ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component) override;
	virtual void ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component) override;

	UFUNCTION()
	virtual void OnTriggerEvent(const bool bOverlapping, UFlowComponent* OtherFlowComponent);
};
