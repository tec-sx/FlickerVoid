#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVFactDebugSubsystem.generated.h"

UCLASS()
class UFVFactDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Facts"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
