#pragma once

#include "FVDebugHUDSubsystem.h"
#include "FVInteractionDebugSubsystem.generated.h"

class UFVInteractorComponent;

/** Interaction readout and detection drawing for the local player's interactor, read from its debug state. */
UCLASS()
class UFVInteractionDebugSubsystem : public UFVDebugHUDSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool IsEnabled() const override;
	virtual FString GetTitle() const override { return TEXT("Interaction"); }
	virtual void CollectLines(TArray<FString>& OutLines) const override;

private:
	const UFVInteractorComponent* FindPlayerInteractor() const;
	void HandlePostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds) const;
	void DrawRange(const UFVInteractorComponent& Interactor, int32 DrawLevel) const;
	void DrawTrace(const UFVInteractorComponent& Interactor) const;

	FDelegateHandle PostActorTickHandle;
};
