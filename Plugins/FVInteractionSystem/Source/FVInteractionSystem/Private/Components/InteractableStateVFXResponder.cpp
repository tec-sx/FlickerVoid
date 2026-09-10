#include "Components/InteractableStateVFXResponder.h"

#include "Components/InteractableComponent.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractableStateVFXResponder)

void UInteractableStateVFXResponder::BindInteractableResponses_Implementation(UInteractableComponent* Interactable)
{
	Interactable->OnStateChanged.AddDynamic(this, &UInteractableStateVFXResponder::HandleStateChanged);
}

void UInteractableStateVFXResponder::HandleStateChanged(EInteractableState NewState)
{
	const FInteractableStateEffect* Entry = StateEffects.FindByPredicate([NewState](const FInteractableStateEffect& Candidate)
	{
		return Candidate.State == NewState;
	});

	if (!Entry)
	{
		return;
	}

	if (Entry->Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Entry->Sound, GetOwner()->GetActorLocation());
	}

	OnStateEffect(NewState, Entry->Effect);
}
