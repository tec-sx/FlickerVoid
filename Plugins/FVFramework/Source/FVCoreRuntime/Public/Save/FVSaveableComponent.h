#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVSaveableComponent.generated.h"

UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVCORERUNTIME_API UFVSaveableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVSaveableComponent();

	UFUNCTION(BlueprintPure, Category = "FV|Save")
	FGuid GetSaveId() const { return SaveId; }

	void WriteActorData(TArray<uint8>& OutData) const;
	void ReadActorData(const TArray<uint8>& InData) const;

#if WITH_EDITOR
	virtual void OnComponentCreated() override;
	virtual void PostEditImport() override;
#endif

private:
	UPROPERTY(VisibleAnywhere, Category = "Save", DuplicateTransient)
	FGuid SaveId;
};
