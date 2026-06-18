#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactions/FVInteractionTypes.h"
#include "FVInteractionInstigatorComponent.generated.h"

class UFVInteractionSubsystem;
class AFVCharacter;
class UFVInteractionInstigatorConfig;
class UFVInteractionTargetComponent;

//~=============================================================================
// Player-side interaction manager.
//
// Responsibilities:
//   - Sphere sweep each tick to find the best interactable in focus
//   - Broadcast focus changes to the UI
//   - Route input tag calls (E press, F hold, etc.) to the focused interactable
//~=============================================================================

UCLASS(Blueprintable, ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractionInstigatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionInstigatorComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime, 
		ELevelTick TickType, 
		FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interaction)
	float DetectionRadius = 350.f;

	// Cosine of the half-angle of the cone in which an interactable can be focused
	// (0 = 90°, 0.5 = ~60°, 0.707 = 45°). Interactables outside this cone are ignored.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "1"), Category=Interaction)
	float DetectionConeAngle = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"), Category=Interaction)
	float DetectionUpdateInterval = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Interaction)
	TArray<TEnumAsByte<EObjectTypeQuery>> DetectionObjectTypes;
	
	UFUNCTION()
	UFVInteractionSubsystem* GetInteractionSubsystem() const;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> Owner;
	
	bool bIsInitialized = false;

	mutable TWeakObjectPtr<UFVInteractionSubsystem> InteractionSubsystem;
	float TimeSinceLastUpdate = 0.f;
	
	void DetectInteractables() const;
	UFVInteractionTargetComponent* FindBestTarget() const;
};
