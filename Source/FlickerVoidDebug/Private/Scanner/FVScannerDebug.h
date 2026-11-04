#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVScannerDebug.generated.h"

/** Shows scan mode, the focused scannable and scan progress while FVCvar.Scanner.Debug.HUD is on. */
UCLASS()
class UFVScannerDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Scanner"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
