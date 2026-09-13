#include "Abilities/FVInteractAbility.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractAbility)

UFVInteractorComponent* UFVInteractAbility::GetInteractor(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : GetAvatarActorFromActorInfo();

	return Avatar ? Avatar->FindComponentByClass<UFVInteractorComponent>() : nullptr;
}

UFVInteractorComponent* UFVInteractAbility::GetInteractor() const
{
	return GetInteractor(CurrentActorInfo);
}

UFVInteractableComponent* UFVInteractAbility::GetInteractable() const
{
	const UFVInteractorComponent* Interactor = GetInteractor(CurrentActorInfo);

	return Interactor ? Interactor->GetFocusedTarget() : nullptr;
}
