#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVRelevanceDebugSubsystem.generated.h"

UCLASS()
class UFVRelevanceDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Relevance"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;
};
