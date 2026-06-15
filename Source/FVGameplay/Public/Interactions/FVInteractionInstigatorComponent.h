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
	
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UFVInteractionInstigatorConfig> Config;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	EFVInteractionResult RequestInteraction(const FGameplayTag& InputTag);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelCurrentInteraction() const;
	
	UFUNCTION()
	UFVInteractionSubsystem* GetInteractionSubsystem() const;
	
	UFUNCTION()
	FGameplayTagContainer& GetTags() { return InstigatorTags; }
	
	UFUNCTION()
	void AddTag(const FGameplayTag& Tag) { InstigatorTags.AddTagFast(Tag); }
	
	UFUNCTION()
	void  RemoveTag(const FGameplayTag& Tag) { InstigatorTags.RemoveTag(Tag); }
	
private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> Owner;
	
	UPROPERTY(Transient)
	FGameplayTagContainer InstigatorTags;
	
	bool bIsInitialized = false;

	mutable TWeakObjectPtr<UFVInteractionSubsystem> InteractionSubsystem;
	float TimeSinceLastUpdate = 0.f;
	
	void UpdateFocus() const;
	UFVInteractionTargetComponent* FindBestTarget() const;
};
