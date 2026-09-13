#pragma once

#include "CoreMinimal.h"
#include "Components/FVInteractorResponseComponent.h"
#include "FVInteractorResponseComponent_ActivateAbility.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

struct FFVInteractionCommit;

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVInteractorResponseComponent_ActivateAbility final : public UFVInteractorResponseComponent
{
	GENERATED_BODY()

protected:
	virtual void BindEvents_Implementation(UFVInteractorComponent* Interactor) override;

private:
	UFUNCTION()
	void OnInteractionRequested(const FFVInteractionCommit& Commit, bool bSuccess) const;
};

#undef UE_API
