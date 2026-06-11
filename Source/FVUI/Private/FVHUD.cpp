#include "FVHUD.h"
#include "Interaction/FVInteractionPromptWidget.h"
#include "Blueprint/UserWidget.h"

void AFVHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (InteractionPromptWidgetClass)
	{
		InteractionPromptWidget = CreateWidget<UFVInteractionPromptWidget>(PC, InteractionPromptWidgetClass);
		if (InteractionPromptWidget)
		{
			InteractionPromptWidget->AddToViewport(0);
		}
	}

	if (DialogueWidgetClass)
	{
		DialogueWidget = CreateWidget<UUserWidget>(PC, DialogueWidgetClass);
		if (DialogueWidget)
		{
			DialogueWidget->AddToViewport(1);
		}
	}
}
