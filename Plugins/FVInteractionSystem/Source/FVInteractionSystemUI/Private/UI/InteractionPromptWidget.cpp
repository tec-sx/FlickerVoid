#include "UI/InteractionPromptWidget.h"

#include "Components/InteractorComponent.h"
#include "FVInteractionSystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionPromptWidget)

void UInteractionPromptWidget::BindToInteractor(UInteractorComponent* InInteractor)
{
	UnbindFromInteractor();

	Interactor = InInteractor;

	if (InInteractor)
	{
		InInteractor->OnOffersChanged.AddDynamic(this, &UInteractionPromptWidget::HandleOffersChanged);
		HandleOffersChanged(InInteractor->GetPrompts());
	}
}

void UInteractionPromptWidget::UnbindFromInteractor()
{
	if (UInteractorComponent* Bound = Interactor.Get())
	{
		Bound->OnOffersChanged.RemoveDynamic(this, &UInteractionPromptWidget::HandleOffersChanged);
	}

	Interactor = nullptr;
}

void UInteractionPromptWidget::NativeDestruct()
{
	UnbindFromInteractor();

	Super::NativeDestruct();
}

bool UInteractionPromptWidget::ResolveKeyBinding(FGameplayTag InputTag, FInteractionKeyBinding& OutBinding) const
{
	if (const FInteractionKeyBinding* Binding = GetDefault<UFVInteractionSystemSettings>()->FindInputBinding(InputTag))
	{
		OutBinding = *Binding;
		return true;
	}

	OutBinding = FInteractionKeyBinding();
	return false;
}

void UInteractionPromptWidget::HandleOffersChanged(const TArray<FInteractionPrompt>& Prompts)
{
	SetVisibility(Prompts.Num() > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	OnPromptsUpdated(Prompts);
}
