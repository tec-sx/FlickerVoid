#include "Components/FVInteractionUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "Components/FVInteractorComponent.h"
#include "Data/FVInteractionUISettings.h"
#include "Engine/LocalPlayer.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Layout/FVUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionUIComponent)

UFVInteractionUIComponent::UFVInteractionUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const UFVInteractionUISettings* UFVInteractionUIComponent::GetUISettings() const
{
	if (UISettingsOverride)
	{
		return UISettingsOverride;
	}
	return UFVInteractionSystemSettings::Get().InteractionUISettings.LoadSynchronous();
}

void UFVInteractionUIComponent::BeginPlay()
{
	Super::BeginPlay();

	Interactor = GetOwner()->FindComponentByClass<UFVInteractorComponent>();
	if (!ensureMsgf(Interactor, TEXT("%s on %s requires a UFVInteractorComponent on the same actor."), *GetName(), *GetNameSafe(GetOwner())))
	{
		UE_LOG(LogFVInteraction, Error, TEXT("%s on %s has no UFVInteractorComponent."), *GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	Interactor->InteractableFound.AddDynamic(this, &UFVInteractionUIComponent::OnFocusChanged);
	Interactor->OffersChanged.AddDynamic(this, &UFVInteractionUIComponent::OnOffersChanged);
	Interactor->InteractionCommitProgressed.AddDynamic(this, &UFVInteractionUIComponent::OnInteractionProgress);

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UFVInteractionUIComponent::OnControllerChanged);
	}

	ShowWidget();
}

void UFVInteractionUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Interactor)
	{
		Interactor->InteractableFound.RemoveDynamic(this, &UFVInteractionUIComponent::OnFocusChanged);
		Interactor->OffersChanged.RemoveDynamic(this, &UFVInteractionUIComponent::OnOffersChanged);
		Interactor->InteractionCommitProgressed.RemoveDynamic(this, &UFVInteractionUIComponent::OnInteractionProgress);
		Interactor = nullptr;
	}

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &UFVInteractionUIComponent::OnControllerChanged);
	}

	HideWidget();

	Super::EndPlay(EndPlayReason);
}

void UFVInteractionUIComponent::ShowWidget()
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	ULocalPlayer* LocalPlayer = PC && PC->IsLocalController() ? PC->GetLocalPlayer() : nullptr;
	if (Widget || !LocalPlayer)
	{
		return;
	}

	const UFVInteractionUISettings* Settings = GetUISettings();
	if (!Settings)
	{
		UE_LOG(LogFVInteraction, Warning, TEXT("No Interaction UI Settings assigned; prompt widget will not be shown."));
		return;
	}

	UFVUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UFVUIManagerSubsystem>();
	if (UIManager)
	{
		Widget = UIManager->PushUserWidget(Settings->LayerTag, Settings->WidgetClass.LoadSynchronous());
	}
}

void UFVInteractionUIComponent::HideWidget()
{
	if (!Widget)
	{
		return;
	}

	if (const ULocalPlayer* LocalPlayer = Widget->GetOwningLocalPlayer())
	{
		if (UFVUIManagerSubsystem* UIManager = LocalPlayer->GetSubsystem<UFVUIManagerSubsystem>())
		{
			UIManager->PopUserWidget(Widget);
		}
	}
	Widget = nullptr;
}

void UFVInteractionUIComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	HideWidget();
	ShowWidget();
}

void UFVInteractionUIComponent::OnFocusChanged(UFVInteractableComponent* NewTarget)
{
	FocusedTarget = NewTarget;

	if (!FocusedTarget)
	{
		Offers.Reset();
		OffersChanged.Broadcast(Offers);
	}
}

void UFVInteractionUIComponent::OnOffersChanged(const TArray<FFVInteractionOfferData>& InOffers)
{
	Offers = InOffers;
	OffersChanged.Broadcast(Offers);
}

void UFVInteractionUIComponent::OnInteractionProgress(const FFVInteractionCommit& Commit, float Progress)
{
	OfferProgress.Broadcast(Commit.ActionTag, Progress);
}
