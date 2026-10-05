#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVInventoryDebugSubsystem.generated.h"

/** Shows the player's carried items, weight and space while FVCvar.Inventory.Debug.HUD is on. */
UCLASS()
class UFVInventoryDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Inventory"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
