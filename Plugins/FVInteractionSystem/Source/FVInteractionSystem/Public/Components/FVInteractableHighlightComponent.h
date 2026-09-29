#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FVInteractionTypes.h"
#include "FVInteractionSystemSettings.h"

#include "FVInteractableHighlightComponent.generated.h"

class UFVInteractableComponent;
class UFVInteractorComponent;
class UMaterialInterface;
class UPrimitiveComponent;

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class FVINTERACTIONSYSTEM_API UFVInteractableHighlightComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractableHighlightComponent() { PrimaryComponentTick.bCanEverTick = false; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight", meta = (Tooltip = "Use this setup instead of the project default."))
	bool bOverrideSetup = false;

	UPROPERTY(EditAnywhere, Category = "Interaction|Highlight", meta = (ShowOnlyInnerProperties))
	FFVInteractionHighlightSetup HighlightSetup;

private:
	UFUNCTION()
	void OnInteractorFound(UFVInteractorComponent* Interactor);
	
	UFUNCTION()
	void OnInteractorLost(UFVInteractorComponent* Interactor);
	
	void CacheTargets();
	void RenderHighlight(const bool bIsInFocus);
	
	TWeakObjectPtr<UFVInteractableComponent> Interactable;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPrimitiveComponent>> Targets;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> CachedOverlays;

	TArray<bool> CachedRenderCustomDepth;
	TArray<int32> CachedStencilValues;
	bool bIsHighlighted = false;
};
