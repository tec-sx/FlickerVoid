#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVSaveableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnActorDataLoaded);

/**
 * Saves the SaveGame properties of its actor and the actor's components. Placed actors get a stable Save Id;
 * actors spawned at runtime have none and are skipped by the save pipeline.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVCORERUNTIME_API UFVSaveableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVSaveableComponent();

	static UFVSaveableComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Save")
	FGuid GetSaveId() const { return SaveId; }

	UFUNCTION(BlueprintCallable, Category = "FV|Save")
	void WriteActorData(TArray<uint8>& OutData) const;

	/** Restores the data, then broadcasts OnActorDataLoaded so components rebuild their runtime state. */
	UFUNCTION(BlueprintCallable, Category = "FV|Save")
	void ReadActorData(const TArray<uint8>& InData);

	UPROPERTY(BlueprintAssignable, Category = "FV|Save")
	FFVOnActorDataLoaded OnActorDataLoaded;

#if WITH_EDITOR
	virtual void OnComponentCreated() override;
	virtual void PostEditImport() override;
#endif

private:
	UPROPERTY(VisibleAnywhere, Category = "Save", DuplicateTransient)
	FGuid SaveId;
};
