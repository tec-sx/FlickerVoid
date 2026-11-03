#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVScannerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanModeChanged, bool, bActive);

/** Global scan mode state, for post process, audio and UI. Scannables in range come from the relevance system. */
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

private:
	bool bScanModeActive = false;
};
