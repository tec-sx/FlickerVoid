#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/FVInteractionInfo.h"
#include "FVInteractionTypes.generated.h"

class UFVInteractionTargetComponent;

// Result of a single interaction attempt
UENUM(BlueprintType)
enum class EFVInteractionResult : uint8
{
	Success,
	RequirementNotMet,	// Player lacks required tags / attributes / items
	Blocked,			// Handler is already running or interactable is locked
	NotOffered,			// No interaction bound to that slot on the current interactable
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

// Why a running interaction was cancelled. Tasks read this in ExitState to
// decide whether to roll back, play a reaction, or fail silently.
UENUM(BlueprintType)
enum class EFVInteractionCancelReason : uint8
{
	None,
	WalkedAway,
	HigherPriorityOffer,
	OfferExpired,
	CombatStarted,
	Death,
	Scripted,
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
	TObjectPtr<UFVInteractionTargetComponent> TargetComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector InteractionPoint = FVector::ZeroVector;
};

UENUM(BlueprintType)
enum class EFVInteractionFocusProfile : uint8
{
	Precise,
	Loose,
	Custom,
};

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVInteractionFocusProfile
{
	GENERATED_BODY()

	// Cosine of the half-angle of the focus cone (0 = 90°, 0.5 = ~60°, 0.85 = ~32°).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "1"))
	float ConeCosine = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float AngularWeight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float DistanceWeight = 0.2f;

	static FFVInteractionFocusProfile Precise()
	{
		FFVInteractionFocusProfile Profile;
		Profile.ConeCosine = 0.85f;
		Profile.AngularWeight = 1.f;
		Profile.DistanceWeight = 0.2f;
		return Profile;
	}

	static FFVInteractionFocusProfile Loose()
	{
		FFVInteractionFocusProfile Profile;
		Profile.ConeCosine = 0.35f;
		Profile.AngularWeight = 0.7f;
		Profile.DistanceWeight = 0.5f;
		return Profile;
	}
};

// Broadcast delegate used to push display data to the UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionFocusChanged, UFVInteractionTargetComponent*, Target);

// Fired by a handler when its execution is done (sync or async)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnInteractionCompleted,
	const FFVInteractionContext&, Context,
	EFVInteractionStatus, Status,
	bool, bSuccess);