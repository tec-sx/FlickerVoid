#include "Abilities/FVInteractAbility.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractAbility)

UInteractorComponent* UFVInteractAbility::GetInteractor(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : GetAvatarActorFromActorInfo();

	return Avatar ? Avatar->FindComponentByClass<UInteractorComponent>() : nullptr;
}

UInteractorComponent* UFVInteractAbility::GetInteractor() const
{
	return GetInteractor(CurrentActorInfo);
}

UInteractableComponent* UFVInteractAbility::GetInteractable() const
{
	const UInteractorComponent* Interactor = GetInteractor(CurrentActorInfo);

	return Interactor ? Interactor->GetFocusedTarget() : nullptr;
}
