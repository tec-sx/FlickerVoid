#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVSocialDebugSubsystem.generated.h"

/** Shows standing, fame, notoriety and titles while FVCvar.Social.Debug.HUD is on. */
UCLASS()
class UFVSocialDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Social"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
