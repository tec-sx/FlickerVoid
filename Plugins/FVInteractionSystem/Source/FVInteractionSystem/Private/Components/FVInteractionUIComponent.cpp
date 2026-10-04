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

	CacheCrosshairs();
	CurrentCrosshair = DefaultCrosshair;
	bReticleVisible = Interactor->GetActiveDetection().bShowReticle;

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
		InteractionWidget->OnCrosshairChanged(CurrentCrosshair, CurrentInteractableType);
		InteractionWidget->OnReticleVisibilityChanged(bReticleVisible);
		InteractionWidget->OnOffersChanged(Offers);
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

UFVInteractionWidget* UFVInteractionUIComponent::GetInteractionWidget() const
{
	return Cast<UFVInteractionWidget>(Widget);
}

void UFVInteractionUIComponent::CacheCrosshairs()
{
	CrosshairOverrides.Reset();
	const UFVInteractionUISettings* Settings = GetUISettings();
	if (!Settings)
	{
		return;
	}

	DefaultCrosshair = Settings->DefaultCrosshair.LoadSynchronous();

	if (const UDataTable* Table = Settings->CrosshairOverrides.LoadSynchronous())
	{
		Table->ForeachRow<FFVInteractionCrosshairRow>(TEXT("FVInteractionCrosshair"), [this](const FName& RowName, const FFVInteractionCrosshairRow& Row)
		{
			if (UTexture2D* Icon = Row.Icon.LoadSynchronous())
			{
				CrosshairOverrides.Add(RowName, Icon);
			}
		});
	}
}

UTexture2D* UFVInteractionUIComponent::ResolveCrosshair(const FGameplayTag& InteractableType) const
{
	for (FGameplayTag Tag = InteractableType; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const TObjectPtr<UTexture2D>* Found = CrosshairOverrides.Find(Tag.GetTagName()))
		{
			return *Found;
		}
	}
	return DefaultCrosshair;
}

void UFVInteractionUIComponent::UpdateCrosshair()
{
	const FGameplayTag NewType = FocusedTarget ? FocusedTarget->GetInteractableType() : FGameplayTag();
	UTexture2D* NewCrosshair = ResolveCrosshair(NewType);
	if (NewCrosshair == CurrentCrosshair && NewType == CurrentInteractableType)
	{
		return;
	}

	CurrentCrosshair = NewCrosshair;
	CurrentInteractableType = NewType;

	if (UFVInteractionWidget* InteractionWidget = GetInteractionWidget())
	{
		InteractionWidget->OnCrosshairChanged(CurrentCrosshair, CurrentInteractableType);
	}
}

void UFVInteractionUIComponent::UpdateReticle()
{
	const bool bNewVisible = Interactor && Interactor->GetActiveDetection().bShowReticle;
	if (bNewVisible == bReticleVisible)
	{
		return;
	}

	bReticleVisible = bNewVisible;

	if (UFVInteractionWidget* InteractionWidget = GetInteractionWidget())
	{
		InteractionWidget->OnReticleVisibilityChanged(bReticleVisible);
	}
}

void UFVInteractionUIComponent::OnModeChanged(FGameplayTag NewMode, FGameplayTag OldMode)
{
	UpdateReticle();
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

	if (Bounds.IsValid)
	{
		return Bounds.GetCenter();
	}

	const AActor* TargetActor = FocusedTarget->GetOwner();
	return TargetActor ? TargetActor->GetActorLocation() : FVector::ZeroVector;
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
	UpdateCrosshair();

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
