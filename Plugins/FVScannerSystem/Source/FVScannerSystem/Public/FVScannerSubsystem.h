#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVScannerSubsystem.generated.h"

class UFVScannableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanModeChanged, bool, bActive);

/** Registry of scannables and the global scan mode state (for post process and audio). */
UCLASS()
class FVSCANNERSYSTEM_API UFVScannerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVScannerSubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	bool IsScanModeActive() const { return bScanModeActive; }

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScanModeChanged OnScanModeChanged;

	void SetScanModeActive(bool bActive);

	void Register(UFVScannableComponent* Scannable) { Scannables.AddUnique(Scannable); }
	void Unregister(UFVScannableComponent* Scannable) { Scannables.Remove(Scannable); }
	const TArray<TWeakObjectPtr<UFVScannableComponent>>& GetScannables() const { return Scannables; }

private:
	TArray<TWeakObjectPtr<UFVScannableComponent>> Scannables;
	bool bScanModeActive = false;
};
