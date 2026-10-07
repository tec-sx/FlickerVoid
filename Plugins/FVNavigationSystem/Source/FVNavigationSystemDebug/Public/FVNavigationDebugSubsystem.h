#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVNavigationDebugSubsystem.generated.h"

/** Shows the active map, the player's map position and the markers around them while FVCvar.Navigation.Debug.HUD is on. */
UCLASS()
class UFVNavigationDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Navigation"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
