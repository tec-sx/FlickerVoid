#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FVMovementSet.generated.h"

class UBaseMovementMode;
class UBaseMovementModeTransition;

/** One named Mover movement mode in a movement set. */
USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVMovementSetMode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Name Mover knows the mode by (Walking, Falling, Swimming...). A later set with the same name replaces it."))
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Movement", meta = (ToolTip = "The mode, C++ or AngelScript, with its settings and its own transitions."))
	TObjectPtr<UBaseMovementMode> Mode;
};

/**
 * A group of Mover movement modes and transitions a character can have, such as base movement, swimming or climbing.
 * Characters list their sets on the movement set component; sets can also be added and removed at runtime.
 */
UCLASS(BlueprintType, Const)
class FLICKERVOIDCHARACTER_API UFVMovementSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (TitleProperty = "Name"))
	TArray<FFVMovementSetMode> Modes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Movement", meta = (ToolTip = "Transitions checked in every mode, such as entering this set's modes from another set's mode."))
	TArray<TObjectPtr<UBaseMovementModeTransition>> Transitions;
};
