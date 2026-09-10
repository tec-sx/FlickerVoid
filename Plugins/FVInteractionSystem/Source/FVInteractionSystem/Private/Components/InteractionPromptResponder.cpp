#include "Components/InteractionPromptResponder.h"

#include "Components/InteractionResponseComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionPromptResponder)

void UInteractionPromptResponder::BindResponses_Implementation(UInteractionResponseComponent* Response)
{
	Response->OnFocusChanged.AddDynamic(this, &UInteractionPromptResponder::HandleFocusChanged);
	Response->OnOffersChanged.AddDynamic(this, &UInteractionPromptResponder::HandleOffersChanged);
	Response->OnInteractionProgress.AddDynamic(this, &UInteractionPromptResponder::HandleInteractionProgress);
}

void UInteractionPromptResponder::HandleFocusChanged(UInteractableComponent* NewTarget)
{
	FocusedTarget = NewTarget;

	if (!FocusedTarget)
	{
		Prompts.Reset();
		OnPromptsChanged.Broadcast(Prompts);
	}
}

void UInteractionPromptResponder::HandleOffersChanged(const TArray<FInteractionOffer>& Offers)
{
	Prompts = Offers;
	OnPromptsChanged.Broadcast(Prompts);
}

void UInteractionPromptResponder::HandleInteractionProgress(const FInteractionCommit& Commit, float Progress)
{
	OnPromptProgress.Broadcast(Commit.ActionTag, Progress);
}
