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
