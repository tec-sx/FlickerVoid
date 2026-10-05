#include "Components/FVInteractionUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractableComponent.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "UI/FVInteractionWidget.h"
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
	Interactor->InteractableLost.AddDynamic(this, &UFVInteractionUIComponent::OnFocusLost);
	Interactor->OffersChanged.AddDynamic(this, &UFVInteractionUIComponent::OnOffersChanged);
	Interactor->InteractionCommitProgressed.AddDynamic(this, &UFVInteractionUIComponent::OnInteractionProgress);
	Interactor->InteractionCommitEnded.AddDynamic(this, &UFVInteractionUIComponent::OnInteractionEnded);
	Interactor->InteractorModeChanged.AddDynamic(this, &UFVInteractionUIComponent::OnModeChanged);

	CacheFocusIndicators();
	CurrentFocusIndicatorBrush = DefaultFocusIndicatorBrush;

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
		Interactor->InteractableLost.RemoveDynamic(this, &UFVInteractionUIComponent::OnFocusLost);
		Interactor->OffersChanged.RemoveDynamic(this, &UFVInteractionUIComponent::OnOffersChanged);
		Interactor->InteractionCommitProgressed.RemoveDynamic(this, &UFVInteractionUIComponent::OnInteractionProgress);
		Interactor->InteractionCommitEnded.RemoveDynamic(this, &UFVInteractionUIComponent::OnInteractionEnded);
		Interactor->InteractorModeChanged.RemoveDynamic(this, &UFVInteractionUIComponent::OnModeChanged);
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

	if (UFVInteractionWidget* InteractionWidget = GetInteractionWidget())
	{
		InteractionWidget->OnInteractionInitialized(Interactor);
		InteractionWidget->OnFocusIndicatorChanged(CurrentFocusIndicatorBrush, CurrentInteractableType);
		InteractionWidget->OnOffersChanged(Offers);
	}

	UpdateOverlay();
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
			if (OverlayWidget)
			{
				UIManager->PopUserWidget(OverlayWidget);
			}
		}
	}
	Widget = nullptr;
	OverlayWidget = nullptr;
	OverlayClass = nullptr;
}

void UFVInteractionUIComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	HideWidget();
	ShowWidget();
}

UFVInteractionWidget* UFVInteractionUIComponent::GetInteractionWidget() const
{
	return Cast<UFVInteractionWidget>(Widget);
}

void UFVInteractionUIComponent::CacheFocusIndicators()
{
	FocusIndicatorOverrides.Reset();
	const UFVInteractionUISettings* Settings = GetUISettings();
	if (!Settings)
	{
		return;
	}

	DefaultFocusIndicatorBrush = Settings->DefaultFocusIndicatorBrush;

	if (const UDataTable* Table = Settings->FocusIndicatorOverrides.LoadSynchronous())
	{
		Table->ForeachRow<FFVInteractionFocusIndicatorRow>(TEXT("FVInteractionFocusIndicator"), [this](const FName& RowName, const FFVInteractionFocusIndicatorRow& Row)
		{
			FocusIndicatorOverrides.Add(RowName, Row.Brush);
		});
	}
}

const FSlateBrush& UFVInteractionUIComponent::ResolveFocusIndicator(const FGameplayTag& InteractableType) const
{
	for (FGameplayTag Tag = InteractableType; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FSlateBrush* Found = FocusIndicatorOverrides.Find(Tag.GetTagName()))
		{
			return *Found;
		}
	}
	return DefaultFocusIndicatorBrush;
}

void UFVInteractionUIComponent::UpdateFocusIndicator()
{
	const FGameplayTag NewType = FocusedTarget ? FocusedTarget->GetInteractableType() : FGameplayTag();
	const FSlateBrush& NewBrush = ResolveFocusIndicator(NewType);
	if (NewBrush == CurrentFocusIndicatorBrush && NewType == CurrentInteractableType)
	{
		return;
	}

	CurrentFocusIndicatorBrush = NewBrush;
	CurrentInteractableType = NewType;

	if (UFVInteractionWidget* InteractionWidget = GetInteractionWidget())
	{
		InteractionWidget->OnFocusIndicatorChanged(CurrentFocusIndicatorBrush, CurrentInteractableType);
	}
}

void UFVInteractionUIComponent::UpdateOverlay()
{
	const UFVInteractionUISettings* Settings = GetUISettings();
	const ULocalPlayer* LocalPlayer = Widget ? Widget->GetOwningLocalPlayer() : nullptr;
	UFVUIManagerSubsystem* UIManager = LocalPlayer ? LocalPlayer->GetSubsystem<UFVUIManagerSubsystem>() : nullptr;
	if (!Interactor || !Settings || !UIManager)
	{
		return;
	}

	const FFVInteractorDetectionSettings& Detection = Interactor->GetActiveDetection();
	const TSubclassOf<UUserWidget> NewClass = Detection.OverlayWidgetClass ? Detection.OverlayWidgetClass : nullptr;
	if (NewClass == OverlayClass)
	{
		return;
	}

	if (OverlayWidget)
	{
		UIManager->PopUserWidget(OverlayWidget);
		OverlayWidget = nullptr;
	}

	OverlayClass = NewClass;
	if (OverlayClass)
	{
		OverlayWidget = UIManager->PushUserWidget(Settings->LayerTag, OverlayClass);
	}
}

void UFVInteractionUIComponent::OnModeChanged(FGameplayTag NewMode, FGameplayTag OldMode)
{
	UpdateOverlay();
}

FVector UFVInteractionUIComponent::GetFocusWorldLocation() const
{
	FBox Bounds(ForceInit);
	for (const UPrimitiveComponent* Primitive : FocusedTarget->GetDetectablePrimitives())
	{
		if (IsValid(Primitive))
		{
			Bounds += Primitive->Bounds.GetBox();
		}
	}

	const AActor* TargetActor = FocusedTarget->GetOwner();
	FVector Location = TargetActor ? TargetActor->GetActorLocation() : FVector::ZeroVector;
	if (Bounds.IsValid)
	{
		Location = Bounds.GetCenter();
		switch (FocusedTarget->GetFocusIndicatorAnchor())
		{
		case EFVFocusIndicatorAnchor::Top: Location.Z = Bounds.Max.Z; break;
		case EFVFocusIndicatorAnchor::Bottom: Location.Z = Bounds.Min.Z; break;
		default: break;
		}
	}

	const FVector Offset = FocusedTarget->GetFocusIndicatorOffset();
	return Location + (TargetActor ? TargetActor->GetActorQuat().RotateVector(Offset) : Offset);
}

bool UFVInteractionUIComponent::GetFocusWidgetPosition(FVector2D& OutPosition) const
{
	OutPosition = FVector2D::ZeroVector;
	if (!FocusedTarget)
	{
		return false;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	return PC && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, GetFocusWorldLocation(), OutPosition, false);
}

void UFVInteractionUIComponent::PushOffersToWidget()
{
	OffersChanged.Broadcast(Offers);
	if (UFVInteractionWidget* InteractionWidget = GetInteractionWidget())
	{
		InteractionWidget->OnOffersChanged(Offers);
	}
}

void UFVInteractionUIComponent::OnFocusChanged(UFVInteractableComponent* NewTarget)
{
	FocusedTarget = NewTarget;
	UpdateFocusIndicator();

	if (!FocusedTarget)
	{
		Offers.Reset();
		PushOffersToWidget();
	}
}

void UFVInteractionUIComponent::OnFocusLost(UFVInteractableComponent* OldTarget)
{
	if (OldTarget == FocusedTarget)
	{
		OnFocusChanged(nullptr);
	}
}

void UFVInteractionUIComponent::OnInteractionEnded(const FFVInteractionCommit& Commit, bool bSuccess)
{
	if (UFVInteractionWidget* InteractionWidget = GetInteractionWidget())
	{
		InteractionWidget->OnOfferEnded(Commit.ActionTag, bSuccess);
	}
}

void UFVInteractionUIComponent::OnOffersChanged(const TArray<FFVInteractionOfferData>& InOffers)
{
	if (FocusedTarget && !FocusedTarget->ShouldShowOffers())
	{
		Offers.Reset();
	}
	else
	{
		Offers = InOffers;
	}
	PushOffersToWidget();
}

void UFVInteractionUIComponent::OnInteractionProgress(const FFVInteractionCommit& Commit, float Progress)
{
	OfferProgress.Broadcast(Commit.ActionTag, Progress);
	if (UFVInteractionWidget* InteractionWidget = GetInteractionWidget())
	{
		InteractionWidget->OnOfferProgress(Commit.ActionTag, Progress);
	}
}
