#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVStoryDebugSubsystem.generated.h"

/** Shows active quests and their objectives while FVCvar.Story.Debug.HUD is on. */
UCLASS()
class UFVStoryDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Quests"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
