#pragma once

#include "Components/InteractionResponseComponent.h"
#include "CoreMinimal.h"

#include "FVInteractionAbilityResponder.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVInteractionAbilityResponder final : public UInteractionResponceComponent
{
	GENERATED_BODY()

protected:
	virtual void BindResponses_Implementation(UInteractionResponseComponent* Response) override;

private:
	UFUNCTION()
	void HandleInteractionRequested(const FInteractionCommit& Commit);
};

#undef UE_API
