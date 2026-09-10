#include "Components/Response_InteractableStateVFX.h"

#include "Components/InteractableComponent.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Response_InteractableStateVFX)

void UResponse_InteractableStateVFX::BindInteractableResponses_Implementation(UInteractableComponent* Interactable)
{
	Interactable->StateChanged.AddDynamic(this, &UResponse_InteractableStateVFX::HandleStateChanged);
}

void UResponse_InteractableStateVFX::HandleStateChanged(EInteractableState NewState)
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
