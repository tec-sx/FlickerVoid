#pragma once
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"

#include "InteractionTypes.generated.h"

class UTexture2D;

class UInteractableComponent;

UENUM(BlueprintType)
enum class EInteractionResult : uint8
{
	Success,
	RequirementNotMet,
	Blocked,
	NoInteractable,
};

UENUM(BlueprintType)
enum class EInteractionStatus : uint8
{
	Idle,
	Executing,
	Completed,
	Failed,
	Cancelled,
};

UENUM(BlueprintType)
enum class EInteractionAvailability : uint8
{
	Available,
	RequirementNotMet,
	Blocked,
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Interactor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Target;

	UPROPERTY(BlueprintReadOnly)
	FVector InteractionPoint = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionFocusProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float DetectionRadius = 10.f;

	// Cosine of the half-angle of the focus cone (0 = 90°, 0.5 = ~60°, 0.85 = ~32°).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1", ClampMax = "1"))
	float ConeCosine = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float AngularWeight = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float DistanceWeight = 0.4f;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionOffer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOffered = true;

	bool IsValid() const { return bOffered && ActionTag.IsValid(); }
};

/** Outcome of asking the owning game whether an action can run right now. */
USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EInteractionAvailability Availability = EInteractionAvailability::RequirementNotMet;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag FailureTag;

	UPROPERTY(BlueprintReadOnly)
	FText FailureText;

	bool operator==(const FInteractionAvailabilityResult& Other) const
	{
		return Availability == Other.Availability && FailureTag == Other.FailureTag;
	}

	bool operator!=(const FInteractionAvailabilityResult& Other) const { return !(*this == Other); }
};

/** Presentation data for one interaction input, authored in project settings. */
USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Glyph;
};

/** Runtime, resolved offer handed to UI. */
USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionPrompt
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly)
	FInteractionAvailabilityResult Result;

	bool IsEnabled() const { return Result.Availability == EInteractionAvailability::Available; }

	bool operator==(const FInteractionPrompt& Other) const
	{
		return InputTag == Other.InputTag && ActionTag == Other.ActionTag && Result == Other.Result;
	}

	bool operator!=(const FInteractionPrompt& Other) const { return !(*this == Other); }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionFocusChanged, UInteractableComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionOffersChanged, const TArray<FInteractionPrompt>&, Prompts);