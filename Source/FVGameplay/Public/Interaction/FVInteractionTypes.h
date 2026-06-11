#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVInteractionTypes.generated.h"

class UFVInteractableComponent;

// Result of a single interaction attempt
UENUM(BlueprintType)
enum class EFVInteractionResult : uint8
{
	Success,
	RequirementNotMet,	// Player lacks required tags / attributes / items
	Blocked,			// Handler is already running or interactable is locked
	ActionNotFound,		// No action with that InputTag on the current interactable
	NoInteractable,		// Nothing is focused
};

// Lifecycle state of a handler that runs over time (mini-games, cutscenes)
UENUM(BlueprintType)
enum class EFVInteractionStatus : uint8
{
	Idle,
	Executing,
	Completed,
	Failed,
	Cancelled,
};

// Full context passed to every handler — everything it could need
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVInteractionContext
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> Instigator;
	
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UFVInteractableComponent> TargetComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector InteractionPoint = FVector::ZeroVector;
};

// Snapshot of a single action for the UI — produced by UFVInteractableComponent
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVInteractionActionDisplay
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	bool bAvailable = true;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FText UnavailableReason;
};

// Broadcast delegate used to push display data to the UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInteractableFocusChanged,
	UFVInteractableComponent*, Interactable,
	const TArray<FFVInteractionActionDisplay>&, Actions);

// Fired by a handler when its execution is done (sync or async)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnInteractionCompleted,
	const FFVInteractionContext&, Context,
	EFVInteractionStatus, Status,
	bool, bSuccess);
