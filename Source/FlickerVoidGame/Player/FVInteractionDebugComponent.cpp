#include "Player/FVInteractionDebugComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionDebugComponent)

#if !UE_BUILD_SHIPPING
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionOfferComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"
#include "Interactions/FVInteractionZoneComponent.h"
#include "Interactions/FVInteractionResolver.h"
#include "DrawDebugHelpers.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"

static TAutoConsoleVariable<bool> CVarInteractionDebugDraw(
	TEXT("FVCvar.Interaction.Debug.Draw"),
	false,
	TEXT("Draw interaction zones, aim probes, cones, and focus/offer highlights"));

static TAutoConsoleVariable<bool> CVarInteractionDebugHUD(
	TEXT("FVCvar.Interaction.Debug.HUD"),
	false,
	TEXT("Show an on-screen readout of interaction candidates, offer, and engagement state"));
#endif

UFVInteractionDebugComponent::UFVInteractionDebugComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if !UE_BUILD_SHIPPING
	PrimaryComponentTick.bCanEverTick = true;
#else
	PrimaryComponentTick.bCanEverTick = false;
#endif
}

#if !UE_BUILD_SHIPPING

void UFVInteractionDebugComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshCachedComponents();

	HUDDrawHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateUObject(this, &UFVInteractionDebugComponent::DrawHUD));
}

void UFVInteractionDebugComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HUDDrawHandle.IsValid())
	{
		UDebugDrawService::Unregister(HUDDrawHandle);
		HUDDrawHandle.Reset();
	}

	Instigator = nullptr;
	OfferComponent = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UFVInteractionDebugComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Instigator.IsValid() || !OfferComponent.IsValid())
	{
		RefreshCachedComponents();
	}

	if (CVarInteractionDebugDraw.GetValueOnGameThread())
	{
		DrawVisualizer();
	}
}

void UFVInteractionDebugComponent::DrawVisualizer() const
{
	UFVInteractionInstigatorComponent* InstigatorPtr = Instigator.Get();
	if (!InstigatorPtr)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UFVInteractionTargetComponent* FocusedTarget = InstigatorPtr->GetFocusedTarget();
	const AActor* OfferActor = OfferComponent.IsValid() && OfferComponent->HasActiveOffer()
		? OfferComponent->GetActiveOffer().Target
			? OfferComponent->GetActiveOffer().Target->GetOwner()
			: nullptr
		: nullptr;

	FVector ViewLocation = InstigatorPtr->GetOwner()->GetActorLocation();
	FVector ViewForward = InstigatorPtr->GetOwner()->GetActorForwardVector();

	if (const APawn* Pawn = Cast<APawn>(InstigatorPtr->GetOwner()))
	{
		if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
			ViewForward = ViewRotation.Vector();
		}
	}

	for (const TWeakObjectPtr<UFVInteractionTargetComponent>& CandidatePtr : InstigatorPtr->GetDebugCandidates())
	{
		UFVInteractionTargetComponent* Target = CandidatePtr.Get();
		if (!Target)
		{
			continue;
		}

		const AActor* TargetOwner = Target->GetOwner();
		if (!TargetOwner)
		{
			continue;
		}

		if (const UFVInteractionZoneComponent* Zone = TargetOwner->FindComponentByClass<UFVInteractionZoneComponent>())
		{
			DrawDebugBox(
				World,
				Zone->GetComponentLocation(),
				Zone->GetScaledBoxExtent(),
				Zone->GetComponentQuat(),
				FColor::Green,
				false, -1.f, 0, 1.5f);
		}

		const FVector ProbeLocation = Target->GetAimProbeLocation();
		const bool bIsFocused = Target == FocusedTarget;
		const bool bIsOffer = TargetOwner == OfferActor;

		const FColor ProbeColor = bIsOffer ? FColor::Magenta : (bIsFocused ? FColor::Yellow : FColor::Cyan);
		DrawDebugSphere(World, ProbeLocation, 6.f, 8, ProbeColor, false, -1.f, 0, 1.5f);

		const FFVInteractionFocusProfile Profile = Target->GetFocusProfile();
		const float HalfAngleRad = FMath::Acos(FMath::Clamp(Profile.ConeCosine, -1.f, 1.f));
		const FVector ToTarget = (ProbeLocation - ViewLocation).GetSafeNormal();

		DrawDebugCone(
			World,
			ViewLocation,
			ToTarget.IsNearlyZero() ? ViewForward : ToTarget,
			Target->GetFocusRadius(),
			HalfAngleRad,
			HalfAngleRad,
			16,
			bIsFocused ? FColor::Yellow : FColor::Silver,
			false, -1.f, 0, 1.f);

		if (bIsFocused)
		{
			DrawDebugLine(World, ViewLocation, ProbeLocation, FColor::Yellow, false, -1.f, 0, 2.f);
		}

		if (bIsOffer)
		{
			DrawDebugSphere(World, TargetOwner->GetActorLocation(), 24.f, 12, FColor::Magenta, false, -1.f, 0, 2.f);
		}
	}
}

void UFVInteractionDebugComponent::DrawHUD(UCanvas* Canvas, APlayerController* PC)
{
	if (!Canvas || !CVarInteractionDebugHUD.GetValueOnGameThread())
	{
		return;
	}

	if (PC != GetOwner())
	{
		return;
	}

	UFVInteractionInstigatorComponent* InstigatorPtr = Instigator.Get();
	if (!InstigatorPtr)
	{
		return;
	}

	FVector ViewLocation = InstigatorPtr->GetOwner()->GetActorLocation();
	FVector ViewForward = InstigatorPtr->GetOwner()->GetActorForwardVector();

	if (const APawn* Pawn = Cast<APawn>(InstigatorPtr->GetOwner()))
	{
		if (const APlayerController* OwnerPC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation;
			OwnerPC->GetPlayerViewPoint(ViewLocation, ViewRotation);
			ViewForward = ViewRotation.Vector();
		}
	}

	const UFVInteractionTargetComponent* FocusedTarget = InstigatorPtr->GetFocusedTarget();

	float X = 20.f;
	float Y = 20.f;
	const float LineHeight = 16.f;
	const FLinearColor HeaderColor = FLinearColor::Yellow;
	const FLinearColor TextColor = FLinearColor::White;

	auto DrawLine = [&](const FString& Text, const FLinearColor& Color)
	{
		Canvas->SetDrawColor(Color.ToFColor(true));
		Canvas->DrawText(GEngine->GetSmallFont(), Text, X, Y);
		Y += LineHeight;
	};

	DrawLine(TEXT("-- Interaction Candidates --"), HeaderColor);

	for (const TWeakObjectPtr<UFVInteractionTargetComponent>& CandidatePtr : InstigatorPtr->GetDebugCandidates())
	{
		const UFVInteractionTargetComponent* Target = CandidatePtr.Get();
		if (!Target)
		{
			continue;
		}

		const FVector ProbeLocation = Target->GetAimProbeLocation();
		const FVector ToTarget = ProbeLocation - ViewLocation;
		const float Distance = ToTarget.Size();
		const float Dot = FVector::DotProduct(ViewForward, ToTarget.GetSafeNormal());
		const FFVInteractionFocusProfile Profile = Target->GetFocusProfile();

		const float AngularRange = 1.f - Profile.ConeCosine;
		const float AngularQuality = AngularRange > KINDA_SMALL_NUMBER
			? FMath::Clamp((Dot - Profile.ConeCosine) / AngularRange, 0.f, 1.f)
			: 1.f;

		const float FocusRadius = Target->GetFocusRadius();
		const float DistanceQuality = FocusRadius > KINDA_SMALL_NUMBER
			? FMath::Clamp(1.f - Distance / FocusRadius, 0.f, 1.f)
			: 0.f;

		const float Score = Profile.AngularWeight * AngularQuality + Profile.DistanceWeight * DistanceQuality;
		const bool bIsFocused = Target == FocusedTarget;

		const AActor* TargetOwner = Target->GetOwner();
		DrawLine(FString::Printf(TEXT("%s%s: Ang=%.2f Dist=%.2f Score=%.2f"),
			bIsFocused ? TEXT("* ") : TEXT("  "),
			TargetOwner ? *TargetOwner->GetName() : TEXT("?"),
			AngularQuality, DistanceQuality, Score),
			bIsFocused ? HeaderColor : TextColor);
	}

	Y += LineHeight * 0.5f;
	DrawLine(TEXT("-- Active Offer --"), HeaderColor);

	if (OfferComponent.IsValid() && OfferComponent->HasActiveOffer())
	{
		const FFVInteractionOffer& Offer = OfferComponent->GetActiveOffer();
		const AActor* TargetOwner = Offer.Target ? Offer.Target->GetOwner() : nullptr;

		DrawLine(FString::Printf(TEXT("Id=%d Source=%s Priority=%d Target=%s"),
			Offer.OfferId,
			Offer.Source == EFVInteractionOfferSource::Focus ? TEXT("Focus") : TEXT("Scripted"),
			Offer.Priority,
			TargetOwner ? *TargetOwner->GetName() : TEXT("?")),
			TextColor);

		if (Offer.IsTimed())
		{
			DrawLine(FString::Printf(TEXT("TimeRemaining=%.1fs"), Offer.TimeRemaining), TextColor);
		}

		static const TCHAR* SlotNames[] = { TEXT("Primary"), TEXT("Secondary"), TEXT("Hold") };
		for (int32 SlotIndex = 0; SlotIndex < static_cast<int32>(EFVInteractionSlot::MAX); ++SlotIndex)
		{
			const EFVInteractionSlot Slot = static_cast<EFVInteractionSlot>(SlotIndex);
			const FFVResolvedInteraction& Resolved = Offer.Resolved.GetSlot(Slot);

			if (!Resolved.IsBound())
			{
				continue;
			}

			DrawLine(FString::Printf(TEXT("  [%s] %s Available=%s%s"),
				SlotNames[SlotIndex],
				*Resolved.Info.DisplayName.ToString(),
				Resolved.Info.bAvailable ? TEXT("true") : TEXT("false"),
				Resolved.Info.bAvailable ? TEXT("") : *FString::Printf(TEXT(" (%s)"), *Resolved.Info.UnavailableReason.ToString())),
				TextColor);
		}
	}
	else
	{
		DrawLine(TEXT("None"), TextColor);
	}

	Y += LineHeight * 0.5f;
	const bool bIsInteracting = OfferComponent.IsValid() && OfferComponent->IsInteracting();
	const UFVInteractionTargetComponent* EngagedTarget = OfferComponent.IsValid() ? OfferComponent->GetEngagedTarget() : nullptr;
	const AActor* EngagedOwner = EngagedTarget ? EngagedTarget->GetOwner() : nullptr;

	DrawLine(FString::Printf(TEXT("Engaged=%s Target=%s"),
		bIsInteracting ? TEXT("true") : TEXT("false"),
		EngagedOwner ? *EngagedOwner->GetName() : TEXT("none")),
		HeaderColor);
}

void UFVInteractionDebugComponent::RefreshCachedComponents()
{
	const AController* Controller = Cast<AController>(GetOwner());
	if (!Controller)
	{
		return;
	}

	const APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		return;
	}

	Instigator = Pawn->FindComponentByClass<UFVInteractionInstigatorComponent>();
	OfferComponent = Pawn->FindComponentByClass<UFVInteractionOfferComponent>();
}

#endif
