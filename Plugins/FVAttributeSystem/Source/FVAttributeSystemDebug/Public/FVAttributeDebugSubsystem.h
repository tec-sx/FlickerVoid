#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVAttributeDebugSubsystem.generated.h"

/** Shows the player's attributes while FVCvar.Attributes.Debug.HUD is on. */
UCLASS()
class UFVAttributeDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Attributes"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
