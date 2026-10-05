#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVDebugHUDSubsystem.generated.h"

class UCanvas;
class APlayerController;

/** Base for on-screen debug readouts. Subclasses gate on a CVar and fill lines; panels stack down the screen. */
UCLASS(Abstract)
class FVCOREDEBUG_API UFVDebugHUDSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	virtual bool IsEnabled() const { return false; }
	virtual FString GetTitle() const { return GetClass()->GetName(); }
	virtual void CollectLines(TArray<FString>& OutLines) const {}

private:
	void Draw(UCanvas* Canvas, APlayerController* PlayerController);

	FDelegateHandle DrawHandle;
};
