#pragma once

#include "FlowComponent.h"
#include "FVFlowTriggerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FFlowTriggerComponentEvent, 
	const bool, bOverlapping, 
	class UFlowComponent*, OtherFlowComponent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLICKERVOIDCORE_API UFVFlowTriggerComponent : public UFlowComponent
{
	GENERATED_BODY()

public:
	explicit UFVFlowTriggerComponent(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FlowTrigger")
	bool bAutoEnable;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "FlowTrigger")
	bool bOverlapEnabled;

	UPROPERTY(BlueprintAssignable, Category = "FlowTrigger")
	FFlowTriggerComponentEvent OnTriggerEvent;
	
	void EnableOverlap();
	void DisableOverlap();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void NotifyFromGraph(
		const FGameplayTagContainer& NotifyTags, 
		const EFlowNetMode NetMode = EFlowNetMode::Authority) override;
	
private:
	TWeakObjectPtr<UShapeComponent> TriggerZonePtr;
	TWeakObjectPtr<UFlowComponent> FlowComponentPtr;
	
	UFUNCTION()
	void OnComponentBeginOverlap(
		UPrimitiveComponent* InOverlappedComponent, 
		AActor* InOtherActor, 
		UPrimitiveComponent* InOtherComp, 
		int32 InOtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& InSweepResult);
	
	UFUNCTION()
	void OnComponentEndOverlap(
		UPrimitiveComponent* InOverlappedComponent, 
		AActor* InOtherActor, 
		UPrimitiveComponent* InOtherComp, 
		int32 InOtherBodyIndex);
};
