#include "Components/Response_InteractionPrompt.h"

#include "Components/InteractionResponseComponent.h"
#include "Components/InteractionSignalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Response_InteractionPrompt)

void UResponse_InteractionPrompt::BindSignals_Implementation(UInteractionSignalComponent* Signal)
{
	Signal->FocusChanged.AddDynamic(this, &UResponse_InteractionPrompt::OnFocusChanged);
	Signal->OffersChanged.AddDynamic(this, &UResponse_InteractionPrompt::OnOffersChanged);
	Signal->InteractionProgress.AddDynamic(this, &UResponse_InteractionPrompt::OnInteractionProgress);
}

void UResponse_InteractionPrompt::UnbindSignals_Implementation(UInteractionSignalComponent* Signal)
{
	Signal->FocusChanged.RemoveDynamic(this, &UResponse_InteractionPrompt::OnFocusChanged);
	Signal->OffersChanged.RemoveDynamic(this, &UResponse_InteractionPrompt::OnOffersChanged);
	Signal->InteractionProgress.RemoveDynamic(this, &UResponse_InteractionPrompt::OnInteractionProgress);
}

void UResponse_InteractionPrompt::OnFocusChanged(UInteractableComponent* NewTarget)
{
	FocusedTarget = NewTarget;

	if (!FocusedTarget)
	{
		Prompts.Reset();
		OnPromptsChanged.Broadcast(Prompts);
	}
}

void UResponse_InteractionPrompt::OnOffersChanged(const TArray<FInteractionOffer>& Offers)
{
	Prompts = Offers;
	OnPromptsChanged.Broadcast(Prompts);
}

void UResponse_InteractionPrompt::OnInteractionProgress(const FInteractionCommit& Commit, float Progress)
{
	OnPromptProgress.Broadcast(Commit.ActionTag, Progress);
}
