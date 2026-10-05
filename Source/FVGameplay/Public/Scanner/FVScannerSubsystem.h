#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVScannerSubsystem.generated.h"

class UFVScannableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanModeChanged, bool, bActive);

/** Global scan mode state, for post process, audio and UI. Scannables come from the interaction registry. */
UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVScannerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVScannerSubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	bool IsScanModeActive() const { return bScanModeActive; }

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScanModeChanged OnScanModeChanged;

	void SetScanModeActive(bool bActive);

	/** Scannable components on registered interactables within Range of Origin. */
	void CollectScannables(const FVector& Origin, float Range, TArray<UFVScannableComponent*>& OutScannables) const;

private:
	bool bScanModeActive = false;
};
