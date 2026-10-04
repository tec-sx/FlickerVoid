#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "FVLockComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnLockStateChanged, bool, bLocked, AActor*, Instigator);

/**
 * Lock state for an interactable actor (doors, containers, devices).
 * Toggle responses on the same actor read their locked state from here when present.
 */
UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UFVLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVLockComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Lock")
	bool IsLocked() const { return bLocked; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Lock")
	bool CanBePicked() const { return bLocked && bPickable; }

	UFUNCTION(BlueprintPure, Category = "Interaction|Lock")
	float GetDifficulty() const { return Difficulty; }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Lock")
	void SetLocked(bool bInLocked, AActor* Instigator = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Interaction|Lock")
	void Unlock(AActor* Instigator = nullptr) { SetLocked(false, Instigator); }

	UFUNCTION(BlueprintCallable, Category = "Interaction|Lock")
	void Lock(AActor* Instigator = nullptr) { SetLocked(true, Instigator); }

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Lock")
	FFVOnLockStateChanged OnLockStateChanged;

protected:
	UPROPERTY(EditAnywhere, SaveGame, Category = "Interaction|Lock")
	bool bLocked = true;

	UPROPERTY(EditAnywhere, Category = "Interaction|Lock", meta = (Tooltip = "Whether the lock can be opened with the lockpick action."))
	bool bPickable = true;

	UPROPERTY(EditAnywhere, Category = "Interaction|Lock", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Difficulty = 0.5f;
};
