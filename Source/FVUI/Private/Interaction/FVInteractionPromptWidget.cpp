#include "Interaction/FVInteractionPromptWidget.h"
#include "Interaction/FVInteractionComponent.h"
#include "Interaction/FVInteractableComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UFVInteractionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PC = GetOwningPlayer())
	{
		// Re-bind when the player possesses a different pawn
		PC->OnPossessedPawnChanged.AddDynamic(this, &UFVInteractionPromptWidget::HandlePawnChanged);

		// Bind to the current pawn if already possessed
		if (APawn* Pawn = PC->GetPawn())
		{
			UFVInteractionComponent* InteractionComp =Pawn->FindComponentByClass<UFVInteractionComponent>();
			BindToInteractionComponent(InteractionComp);
		}
	}
}

void UFVInteractionPromptWidget::NativeDestruct()
{
	UnbindCurrentComponent();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.RemoveDynamic(this, &UFVInteractionPromptWidget::HandlePawnChanged);
	}

	Super::NativeDestruct();
}

void UFVInteractionPromptWidget::HandlePawnChanged(APawn* /*OldPawn*/, APawn* NewPawn)
{
	UnbindCurrentComponent();

	if (NewPawn)
	{
		BindToInteractionComponent(NewPawn->FindComponentByClass<UFVInteractionComponent>());
	}
	else
	{
		// Player unpossessed — blank the prompt
		OnFocusChanged(nullptr, TArray<FFVInteractionActionDisplay>());
	}
}

void UFVInteractionPromptWidget::BindToInteractionComponent(UFVInteractionComponent* Component)
{
	if (!Component)
	{
		return;
	}

	BoundInteractionComponent = Component;
	Component->OnFocusChanged.AddDynamic(this, &UFVInteractionPromptWidget::HandleFocusChanged);

	// Sync immediately with the current focus state
	if (UFVInteractableComponent* Current = Component->GetFocusedInteractable())
	{
		TArray<FFVInteractionActionDisplay> Actions = Current->GetActionDisplayData(Component->GetOwner());
		OnFocusChanged(Current, Actions);
	}
}

void UFVInteractionPromptWidget::UnbindCurrentComponent()
{
	if (UFVInteractionComponent* Component = BoundInteractionComponent.Get())
	{
		Component->OnFocusChanged.RemoveDynamic(this, &UFVInteractionPromptWidget::HandleFocusChanged);
	}
	BoundInteractionComponent = nullptr;
}

void UFVInteractionPromptWidget::HandleFocusChanged(
	UFVInteractableComponent* FocusedInteractable,
	const TArray<FFVInteractionActionDisplay>& AvailableActions)
{
	OnFocusChanged(FocusedInteractable, AvailableActions);
}
