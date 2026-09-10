#include "Components/Response_InteractionPrompt.h"

#include "Components/InteractionResponseComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Response_InteractionPrompt)

void UResponse_InteractionPrompt::BindResponses_Implementation(UInteractionResponseComponent* Response)
{
	Response->OnFocusChanged.AddDynamic(this, &UResponse_InteractionPrompt::HandleFocusChanged);
	Response->OnOffersChanged.AddDynamic(this, &UResponse_InteractionPrompt::HandleOffersChanged);
	Response->OnInteractionProgress.AddDynamic(this, &UResponse_InteractionPrompt::HandleInteractionProgress);
}

void UResponse_InteractionPrompt::HandleFocusChanged(UInteractableComponent* NewTarget)
{
	FocusedTarget = NewTarget;

	if (!FocusedTarget)
	{
		Prompts.Reset();
		OnPromptsChanged.Broadcast(Prompts);
	}
}

void UResponse_InteractionPrompt::HandleOffersChanged(const TArray<FInteractionOffer>& Offers)
{
	Prompts = Offers;
	OnPromptsChanged.Broadcast(Prompts);
}

void UResponse_InteractionPrompt::HandleInteractionProgress(const FInteractionCommit& Commit, float Progress)
{
	OnPromptProgress.Broadcast(Commit.ActionTag, Progress);
}
