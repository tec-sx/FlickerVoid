#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"

#include "FVInteractionZoneComponent.generated.h"

class UFVInteractionTargetComponent;

// Activation gate for interaction detection; see FVInteractionInstigatorComponent.
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractionZoneComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UFVInteractionZoneComponent();

	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFVInteractionTargetComponent* GetOwnerTarget() const;
};
