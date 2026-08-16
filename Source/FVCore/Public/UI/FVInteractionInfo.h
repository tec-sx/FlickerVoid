#pragma once
#include "GameplayTagContainer.h"
#include "FVInteractionInfo.generated.h"

// Input slot an interaction binds to. Determines which prompt widget slot shows it
// and which input drives it. Independent of how the interaction executes.
UENUM(BlueprintType)
enum class EFVInteractionSlot : uint8
{
	Primary,
	Secondary,
	Hold,

	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCORE_API FFVInteractionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FGameplayTag AbilityTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	EFVInteractionSlot Slot = EFVInteractionSlot::Primary;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	bool bAvailable = true;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Display")
	FText UnavailableReason;
};
