#include "Interactions/StateTree/FVInteractionPerceptionEvaluator.h"

#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionOfferComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"

void UFVInteractionPerceptionEvaluator::TreeStart(FStateTreeExecutionContext& Context)
{
	CacheComponents();
	Sample();

	Super::TreeStart(Context);
}

void UFVInteractionPerceptionEvaluator::TreeStop(FStateTreeExecutionContext& Context)
{
	Instigator = nullptr;
	Offers = nullptr;
	FocusedTarget = nullptr;
	EngagedTarget = nullptr;
	EngagedActor = nullptr;
	bHasFocus = false;
	bHasOffer = false;
	bIsInteracting = false;
	bOfferIsTimed = false;
	OfferTimeRemaining = 0.f;

	Super::TreeStop(Context);
}

void UFVInteractionPerceptionEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	if (!Instigator || !Offers)
	{
		CacheComponents();
	}

	Sample();

	Super::Tick(Context, DeltaTime);
}

void UFVInteractionPerceptionEvaluator::CacheComponents()
{
	if (!InteractionOwner)
	{
		return;
	}

	Instigator = InteractionOwner->FindComponentByClass<UFVInteractionInstigatorComponent>();
	Offers = InteractionOwner->FindComponentByClass<UFVInteractionOfferComponent>();
}

void UFVInteractionPerceptionEvaluator::Sample()
{
	FocusedTarget = Instigator ? Instigator->GetFocusedTarget() : nullptr;
	bHasFocus = FocusedTarget != nullptr;

	if (!Offers)
	{
		bHasOffer = false;
		bIsInteracting = false;
		bOfferIsTimed = false;
		OfferTimeRemaining = 0.f;
		EngagedTarget = nullptr;
		EngagedActor = nullptr;
		return;
	}

	const FFVInteractionOffer& Offer = Offers->GetActiveOffer();
	bHasOffer = Offers->HasActiveOffer();
	bOfferIsTimed = bHasOffer && Offer.IsTimed();
	OfferTimeRemaining = bOfferIsTimed ? Offer.TimeRemaining : 0.f;

	EngagedTarget = Offers->GetEngagedTarget();
	EngagedActor = EngagedTarget ? EngagedTarget->GetOwner() : nullptr;
	bIsInteracting = EngagedTarget != nullptr;
}
