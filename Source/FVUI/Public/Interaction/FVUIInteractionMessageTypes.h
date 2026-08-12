// Pure presentation data for the interaction prompt.
// Contains no UFVInteractionAction*, no component pointers, and no FVGameplay types.

#pragma once

#include "CoreMinimal.h"
#include "UI/FVInteractionActionInfo.h"
#include "FVUIInteractionMessageTypes.generated.h"

/** One prompt slot as the widget should draw it. */
USTRUCT(BlueprintType)
struct FFVUIInteractionSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	EFVInteractionSlot Slot = EFVInteractionSlot::Primary;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bOccupied = false;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FText InputHint;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bEnabled = true;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FText DisabledReason;
};

/** The whole prompt state pushed to the UI on the interaction channel. */
USTRUCT(BlueprintType)
struct FFVUIInteractionPromptMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TArray<FFVUIInteractionSlot> Slots;

	/** True while a timed offer is counting down. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bTimed = false;

	/** Normalized 1 -> 0 countdown for timed offers; 0 when untimed. */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	float TimeRemainingRatio = 0.f;
};
