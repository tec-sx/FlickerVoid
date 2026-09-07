#include "Debug/InteractionDebugComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionDebugComponent)

#if !UE_BUILD_SHIPPING
#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "Subsystems/InteractionRegistrySubsystem.h"
#include "Debug/DebugDrawService.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

static TAutoConsoleVariable<bool> CVarInteractionDebugDraw(
	TEXT("FVCvar.Interaction.Debug.Draw"),
	false,
	TEXT("Draw interaction detection volumes, aim probes, cones and focus highlights"));

static TAutoConsoleVariable<bool> CVarInteractionDebugHUD(
	TEXT("FVCvar.Interaction.Debug.HUD"),
	false,
	TEXT("Show an on-screen readout of interaction candidates and resolved prompts"));
#endif

UInteractionDebugComponent::UInteractionDebugComponent()
{
#if !UE_BUILD_SHIPPING
	PrimaryComponentTick.bCanEverTick = true;
#else
	PrimaryComponentTick.bCanEverTick = false;
#endif
}

#if !UE_BUILD_SHIPPING

void UInteractionDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	RefreshCachedInteractor();

	HUDDrawHandle = UDebugDrawService::Register(
		TEXT("Game"),
		FDebugDrawDelegate::CreateUObject(this, &UInteractionDebugComponent::DrawHUD));
}

void UInteractionDebugComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HUDDrawHandle.IsValid())
	{
		UDebugDrawService::Unregister(HUDDrawHandle);
		HUDDrawHandle.Reset();
	}

	Interactor = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UInteractionDebugComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Interactor.IsValid())
	{
		RefreshCachedInteractor();
	}

	if (CVarInteractionDebugDraw.GetValueOnGameThread())
	{
		DrawVisualizer();
	}
}

void UInteractionDebugComponent::RefreshCachedInteractor()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (const AController* Controller = Cast<AController>(Owner))
	{
		Owner = Controller->GetPawn();
	}

	if (Owner)
	{
		Interactor = Owner->FindComponentByClass<UInteractorComponent>();
	}
}

void UInteractionDebugComponent::GetViewPoint(FVector& OutPawnLocation, FVector& OutViewLocation, FVector& OutForward) const
{
	const UInteractorComponent* InteractorPtr = Interactor.Get();
	const AActor* InteractorOwner = InteractorPtr ? InteractorPtr->GetOwner() : nullptr;

	if (!InteractorOwner)
	{
		OutPawnLocation = FVector::ZeroVector;
		OutViewLocation = FVector::ZeroVector;
		OutForward = FVector::ForwardVector;
		return;
	}

	OutPawnLocation = InteractorOwner->GetActorLocation();
	OutViewLocation = OutPawnLocation;
	OutForward = InteractorOwner->GetActorForwardVector();

	if (const APawn* Pawn = Cast<APawn>(InteractorOwner))
	{
		if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(OutViewLocation, ViewRotation);
			OutForward = ViewRotation.Vector();
		}
	}
}

void UInteractionDebugComponent::DrawVisualizer() const
{
	const UInteractorComponent* InteractorPtr = Interactor.Get();
	const UWorld* World = GetWorld();

	if (!InteractorPtr || !World)
	{
		return;
	}

	const UInteractableComponent* FocusedTarget = InteractorPtr->GetFocusedTarget();

	FVector PawnLocation;
	FVector ViewLocation;
	FVector ViewForward;
	GetViewPoint(PawnLocation, ViewLocation, ViewForward);

	FVector AimOrigin;
	FVector AimForward;
	if (!InteractorPtr->GetAimPoint(AimOrigin, AimForward))
	{
		return;
	}

	DrawDebugCircle(
		World,
		PawnLocation,
		InteractorPtr->MaxDetectionRadius,
		64,
		FColor(0, 128, 255),
		false, -1.f, 0, 0.5f,
		FVector::ForwardVector,
		FVector::RightVector,
		false);

	const UInteractionRegistrySubsystem* Registry = World->GetSubsystem<UInteractionRegistrySubsystem>();
	if (!Registry)
	{
		return;
	}

	for (const UInteractableComponent* Interactable : Registry->GetAll())
	{
		if (!Interactable)
		{
			continue;
		}

		const FVector FocusPoint = Interactable->GetFocusPoint();
		const bool bIsFocused = Interactable == FocusedTarget;
		const bool bInRange = FVector::Dist(FocusPoint, PawnLocation) <= Interactable->DetectionRadius;

		const FColor CandidateColor = bIsFocused
			? FColor::Green
			: (bInRange ? FColor::Yellow : FColor(80, 80, 80));

		DrawDebugCircle(
			World,
			FocusPoint,
			Interactable->DetectionRadius,
			48,
			CandidateColor,
			false, -1.f, 0, bIsFocused ? 1.f : 0.5f,
			FVector::ForwardVector,
			FVector::RightVector,
			false);

		if (bIsFocused)
		{
			if (const AActor* TargetOwner = Interactable->GetOwner())
			{
				FVector BoundsOrigin;
				FVector BoundsExtent;
				TargetOwner->GetActorBounds(true, BoundsOrigin, BoundsExtent);

				DrawDebugBox(World, BoundsOrigin, BoundsExtent, TargetOwner->GetActorQuat(), CandidateColor, false, -1.f, 0, 1.f);
			}
		}
	}

	if (InteractorPtr->HasDebugImpact())
	{
		const FVector SweepDirection = InteractorPtr->GetDebugSweepDirection().GetSafeNormal();
		FVector DiscX;
		FVector DiscY;
		SweepDirection.FindBestAxisVectors(DiscX, DiscY);

		DrawDebugCircle(
			World,
			InteractorPtr->GetDebugImpactPoint(),
			InteractorPtr->AimSweepRadius,
			24,
			InteractorPtr->DidDebugHitOccluder() ? FColor::Red : FColor::Green,
			false, -1.f, 0, 0.5f,
			DiscX,
			DiscY,
			false);

		DrawDebugLine(World, AimOrigin, InteractorPtr->GetDebugImpactPoint(),
			InteractorPtr->DidDebugHitOccluder() ? FColor::Red : FColor::Green, false, -1.f, 0, 0.5f);
	}
}

void UInteractionDebugComponent::DrawHUD(UCanvas* Canvas, APlayerController* PC)
{
	if (!Canvas || !CVarInteractionDebugHUD.GetValueOnGameThread())
	{
		return;
	}

	const UInteractorComponent* InteractorPtr = Interactor.Get();
	const AActor* InteractorOwner = InteractorPtr ? InteractorPtr->GetOwner() : nullptr;

	if (!InteractorOwner)
	{
		return;
	}

	const APawn* Pawn = Cast<APawn>(InteractorOwner);
	const AController* OwningController = Pawn ? Pawn->GetController() : Cast<AController>(InteractorOwner);

	if (PC != OwningController)
	{
		return;
	}

	FVector PawnLocation;
	FVector ViewLocation;
	FVector ViewForward;
	GetViewPoint(PawnLocation, ViewLocation, ViewForward);

	const UInteractableComponent* FocusedTarget = InteractorPtr->GetFocusedTarget();

	const float X = 20.f;
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

	for (const UInteractableComponent* Candidate : InteractorPtr->GetDebugCandidates())
	{
		if (!Candidate)
		{
			continue;
		}

		const float Distance = FVector::Dist(Candidate->GetFocusPoint(), PawnLocation);
		const bool bIsFocused = Candidate == FocusedTarget;
		const bool bInRange = Distance <= Candidate->DetectionRadius;

		const AActor* TargetOwner = Candidate->GetOwner();
		DrawLine(FString::Printf(TEXT("%s%s: Dist=%.0f/%.0f %s"),
			bIsFocused ? TEXT("* ") : TEXT("  "),
			TargetOwner ? *TargetOwner->GetName() : TEXT("?"),
			Distance,
			Candidate->DetectionRadius,
			bInRange ? TEXT("in-range") : TEXT("out-of-range")),
			bIsFocused ? HeaderColor : TextColor);
	}

	Y += LineHeight * 0.5f;
	DrawLine(FString::Printf(TEXT("Sweep: gate=%s %s"),
		InteractorPtr->IsDebugGateOpen() ? TEXT("open") : TEXT("closed"),
		!InteractorPtr->HasDebugImpact()
			? TEXT("no hit")
			: (InteractorPtr->DidDebugHitOccluder() ? TEXT("blocked by occluder") : TEXT("hit interactable"))),
		TextColor);

	Y += LineHeight * 0.5f;
	DrawLine(TEXT("-- Prompts --"), HeaderColor);

	const TArray<FInteraction>& Prompts = InteractorPtr->GetPrompts();

	if (Prompts.IsEmpty())
	{
		DrawLine(TEXT("None"), TextColor);
	}
	else
	{
		for (const FInteraction& Prompt : Prompts)
		{
			DrawLine(FString::Printf(TEXT("  [%s] %s Enabled=%s"),
				*Prompt.InputTag.ToString(),
				*Prompt.ActionTag.ToString(),
				Prompt.bCanExecute ? TEXT("true") : TEXT("false")),
				TextColor);
		}
	}

	Y += LineHeight * 0.5f;

	int32 RegisteredCount = 0;
	if (const UWorld* World = GetWorld())
	{
		if (const UInteractionRegistrySubsystem* Registry = World->GetSubsystem<UInteractionRegistrySubsystem>())
		{
			RegisteredCount = Registry->GetAll().Num();
		}
	}

	const AActor* FocusedOwner = FocusedTarget ? FocusedTarget->GetOwner() : nullptr;
	DrawLine(FString::Printf(TEXT("Focused=%s Candidates=%d Registered=%d Range=%.0f"),
		FocusedOwner ? *FocusedOwner->GetName() : TEXT("none"),
		InteractorPtr->GetDebugCandidates().Num(),
		RegisteredCount,
		InteractorPtr->MaxDetectionRadius),
		HeaderColor);

	const UInteractorComponent::EDebugActionOutcome Outcome = InteractorPtr->GetDebugLastOutcome();

	if (Outcome != UInteractorComponent::EDebugActionOutcome::None)
	{
		const TCHAR* OutcomeText = TEXT("");
		FLinearColor OutcomeColor = TextColor;

		switch (Outcome)
		{
		case UInteractorComponent::EDebugActionOutcome::Succeeded:
			OutcomeText = TEXT("SUCCESS");
			OutcomeColor = FLinearColor::Green;
			break;
		case UInteractorComponent::EDebugActionOutcome::Disabled:
			OutcomeText = TEXT("FAILED (requirement not met)");
			OutcomeColor = FLinearColor::Red;
			break;
		case UInteractorComponent::EDebugActionOutcome::ExecuteFailed:
			OutcomeText = TEXT("FAILED (execute rejected)");
			OutcomeColor = FLinearColor::Red;
			break;
		default:
			OutcomeText = TEXT("FAILED (no prompt)");
			OutcomeColor = FLinearColor::Red;
			break;
		}

		const double Age = FPlatformTime::Seconds() - InteractorPtr->GetDebugLastActionTime();
		DrawLine(FString::Printf(TEXT("Last=%s %s (%.1fs ago)"),
			*InteractorPtr->GetDebugLastInputTag().ToString(),
			OutcomeText,
			Age),
			Age < 2.0 ? OutcomeColor : TextColor);
	}
}

#endif
