#include "Abilities/FVInteractAbility.h"

#include "GameFramework/Actor.h"
#include "Interactions/FVInteractionOfferComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractAbility)

UFVInteractionOfferComponent* UFVInteractAbility::GetOfferComponent() const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();

	return Avatar ? Avatar->FindComponentByClass<UFVInteractionOfferComponent>() : nullptr;
}

UFVInteractionTargetComponent* UFVInteractAbility::GetEngagedTarget() const
{
	const UFVInteractionOfferComponent* Offers = GetOfferComponent();

	return Offers ? Offers->GetEngagedTarget() : nullptr;
}

AActor* UFVInteractAbility::GetEngagedActor() const
{
	const UFVInteractionTargetComponent* Target = GetEngagedTarget();

	return Target ? Target->GetOwner() : nullptr;
}

EFVInteractionResult UFVInteractAbility::TryBeginInteraction(EFVInteractionSlot InSlot)
{
	UFVInteractionOfferComponent* Offers = GetOfferComponent();

	return Offers ? Offers->BeginEngagement(InSlot) : EFVInteractionResult::NoInteractable;
}
