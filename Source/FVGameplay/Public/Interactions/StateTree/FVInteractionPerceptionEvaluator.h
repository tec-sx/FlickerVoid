#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "Interactions/FVInteractionTypes.h"

#include "FVInteractionPerceptionEvaluator.generated.h"

class UFVInteractionInstigatorComponent;
class UFVInteractionOfferComponent;
class UFVInteractionTargetComponent;

//~=============================================================================
// Player State Tree evaluator.
//
// Wraps the instigator and offer components rather than duplicating detection:
// the components remain the single source of truth, and this evaluator only
// republishes their state as bindable properties so transitions can key off
// focus, offers and engagement.
//~=============================================================================

UCLASS(Blueprintable, DisplayName = "FV Interaction Perception")
class FLICKERVOIDGAMEPLAY_API UFVInteractionPerceptionEvaluator : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

public:
	/** Actor owning the interaction components. Bind to the tree's context actor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Context")
	TObjectPtr<AActor> InteractionOwner;

	//~=========================================================================
	// Published output \u2014 bind transitions and tasks to these
	//~=========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	bool bHasFocus = false;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Output")
	bool bHasOffer = false;

	/** True while an interaction the player committed to is still running. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	bool bIsInteracting = false;

	/** True when the active offer counts down and will auto-resolve. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	bool bOfferIsTimed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	float OfferTimeRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	TObjectPtr<UFVInteractionTargetComponent> FocusedTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	TObjectPtr<UFVInteractionTargetComponent> EngagedTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	TObjectPtr<AActor> EngagedActor;

protected:
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

private:
	void CacheComponents();
	void Sample();

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractionInstigatorComponent> Instigator;

	UPROPERTY(Transient)
	TObjectPtr<UFVInteractionOfferComponent> Offers;
};
