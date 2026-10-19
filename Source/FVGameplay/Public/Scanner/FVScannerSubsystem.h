#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVScannerSubsystem.generated.h"

class UFVScannableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanModeChanged, bool, bActive);

/** Global scan mode state, for post process, audio and UI, and the scannables currently in scan range. */
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

	void SetInRange(UFVScannableComponent* Scannable, bool bInRange);

	/** Scannables in the Near band of the Relevance.ScanTarget group. */
	void CollectScannables(TArray<UFVScannableComponent*>& OutScannables) const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFVScannableComponent>> InRange;

	bool bScanModeActive = false;
};
