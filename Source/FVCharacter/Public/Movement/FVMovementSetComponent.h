#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVMovementSetComponent.generated.h"

class UBaseMovementModeTransition;
class UFVMovementSet;
class UMoverComponent;

USTRUCT()
struct FFVInstalledMovementSet
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UFVMovementSet> Set;

	UPROPERTY()
	TArray<FName> ModeNames;

	UPROPERTY()
	TArray<TObjectPtr<UBaseMovementModeTransition>> Transitions;
};

/**
 * Gives the owner's Mover component the movement modes and transitions of its movement sets.
 * The owning pawn installs the starting sets before Mover initializes; more can be added or removed at runtime.
 */
UCLASS(ClassGroup = (FlickerVoid), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDCHARACTER_API UFVMovementSetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static UFVMovementSetComponent* Find(const AActor* Actor);

	/** Copies the starting sets into Mover and adds the transition that runs set transitions. Call before Mover initializes (PreInitializeComponents). */
	void InstallStartingSets();

	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (ToolTip = "Adds the set's modes and transitions to Mover. False when it is already installed or there is no Mover."))
	bool AddMovementSet(const UFVMovementSet* Set);

	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (ToolTip = "Removes the set's modes and transitions from Mover. Don't remove the mode the character is in."))
	bool RemoveMovementSet(const UFVMovementSet* Set);

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool HasMovementSet(const UFVMovementSet* Set) const;

	const TArray<FFVInstalledMovementSet>& GetInstalledSets() const { return InstalledSets; }

protected:
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ToolTip = "Sets installed when the character spawns, in order; a later set replaces modes of the same name."))
	TArray<TObjectPtr<const UFVMovementSet>> StartingSets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Mode the character starts in. Empty keeps Mover's Starting Movement Mode."))
	FName StartingMovementMode;

private:
	UMoverComponent* GetMover() const;
	void Install(UMoverComponent& Mover, const UFVMovementSet& Set, bool bMoverInitialized);

	UPROPERTY(Transient)
	TArray<FFVInstalledMovementSet> InstalledSets;
};
