#pragma once
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"

#include "InteractionTypes.generated.h"

class UTexture2D;

class UInteractableComponent;
class UInteractionRequirement;

UENUM(BlueprintType)
enum class EInteractionStatus : uint8
{
	Idle,
	Executing,
	Completed,
	Failed,
	Cancelled,
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
struct FVINTERACTIONSYSTEM_API FInteractionOffer
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOffered = true;

	UPROPERTY(EditAnywhere, Instanced, Category = "Interaction")
	TArray<TObjectPtr<UInteractionRequirement>> Requirements;

	bool IsValid() const { return InputTag.IsValid() && bOffered && ActionTag.IsValid(); }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Glyph;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionPrompt
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly)
	bool bEnabled = false;

	bool IsEnabled() const { return bEnabled; }

	bool operator==(const FInteractionPrompt& Other) const
	{
		return InputTag == Other.InputTag && ActionTag == Other.ActionTag && bEnabled == Other.bEnabled;
	}

	bool operator!=(const FInteractionPrompt& Other) const { return !(*this == Other); }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionFocusChanged, UInteractableComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionOffersChanged, const TArray<FInteractionPrompt>&, Prompts);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionExecuted, const FGameplayTag&, ActionTag);