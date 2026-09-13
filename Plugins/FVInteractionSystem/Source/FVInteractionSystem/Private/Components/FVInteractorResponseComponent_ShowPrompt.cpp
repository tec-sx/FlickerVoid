#include "Components/FVInteractorResponseComponent_ShowPrompt.h"

#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractorResponseComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorResponseComponent_ShowPrompt)

void UFVInteractorResponseComponent_ShowPrompt::BindEvents_Implementation(UFVInteractorComponent* Interactor)
{
	Interactor->FocusChanged.AddDynamic(this, &UFVInteractorResponseComponent_ShowPrompt::OnFocusChanged);
	Interactor->OffersChanged.AddDynamic(this, &UFVInteractorResponseComponent_ShowPrompt::OnOffersChanged);
	Interactor->InteractionCommitProgress.AddDynamic(this, &UFVInteractorResponseComponent_ShowPrompt::OnInteractionProgress);
}

void UFVInteractorResponseComponent_ShowPrompt::UnbindEvents_Implementation(UFVInteractorComponent* Interactor)
{
	Interactor->FocusChanged.RemoveDynamic(this, &UFVInteractorResponseComponent_ShowPrompt::OnFocusChanged);
	Interactor->OffersChanged.RemoveDynamic(this, &UFVInteractorResponseComponent_ShowPrompt::OnOffersChanged);
	Interactor->InteractionCommitProgress.RemoveDynamic(this, &UFVInteractorResponseComponent_ShowPrompt::OnInteractionProgress);
}

void UFVInteractorResponseComponent_ShowPrompt::OnFocusChanged(UFVInteractableComponent* NewTarget)
{
	FocusedTarget = NewTarget;

	if (!FocusedTarget)
	{
		Prompts.Reset();
		OnPromptsChanged.Broadcast(Prompts);
	}
}

void UFVInteractorResponseComponent_ShowPrompt::OnOffersChanged(const TArray<FFVInteractionOffer>& Offers)
{
	Prompts = Offers;
	OnPromptsChanged.Broadcast(Prompts);
}

void UFVInteractorResponseComponent_ShowPrompt::OnInteractionProgress(const FFVInteractionCommit& Commit, float Progress)
{
	OnPromptProgress.Broadcast(Commit.ActionTag, Progress);
}
