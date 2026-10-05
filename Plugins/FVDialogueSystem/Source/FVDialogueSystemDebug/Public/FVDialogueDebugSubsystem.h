#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVDialogueDebugSubsystem.generated.h"

/** Shows the running conversation while FVCvar.Dialogue.Debug.HUD is on. */
UCLASS()
class UFVDialogueDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Dialogue"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
