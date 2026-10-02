#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVInteractionAbilityComponent.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

class UFVInteractorComponent;
struct FFVInteractionCommit;

UCLASS(MinimalAPI, ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent, RequiresInteractor))
class UFVInteractionAbilityComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionAbilityComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnInteractionCommited(const FFVInteractionCommit& Commit, bool bSuccess);

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractorComponent> Interactor;
};

#undef UE_API
