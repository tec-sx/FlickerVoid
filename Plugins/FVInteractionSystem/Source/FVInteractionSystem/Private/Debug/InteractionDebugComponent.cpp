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

static TAutoConsoleVariable<float> CVarInteractionDebugConeLength(
	TEXT("FVCvar.Interaction.Debug.ConeLength"),
	200.f,
	TEXT("Display length of the interaction aim cone, visual only"));
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
	InteractorPtr->GetAimPoint(AimOrigin, AimForward);

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

	float ConeHalfAngleRad = 0.f;

	for (const UInteractableComponent* Candidate : InteractorPtr->GetDebugCandidates())
	{
		if (!Candidate)
		{
			continue;
		}

		const FInteractionFocusProfile& Profile = Candidate->GetFocusProfile();
		const FVector FocusPoint = Candidate->GetFocusPoint();
		const FVector ClosestPoint = Candidate->GetClosestFocusPoint(AimOrigin);
		const bool bIsFocused = Candidate == FocusedTarget;
		const bool bInRange = FVector::Dist(ClosestPoint, PawnLocation) <= Profile.DetectionRadius;

		const float HalfAngleRad = FMath::Acos(FMath::Clamp(Profile.ConeCosine, -1.f, 1.f));
		if (bIsFocused)
		{
			ConeHalfAngleRad = HalfAngleRad;
		}
		else if (ConeHalfAngleRad <= 0.f || !FocusedTarget)
		{
			ConeHalfAngleRad = FMath::Max(ConeHalfAngleRad, HalfAngleRad);
		}

		const FColor CandidateColor = bIsFocused
			? FColor::Yellow
			: (bInRange ? FColor::Green : FColor(80, 80, 80));

		if (const AActor* TargetOwner = Candidate->GetOwner())
		{
			FVector BoundsOrigin;
			FVector BoundsExtent;
			TargetOwner->GetActorBounds(true, BoundsOrigin, BoundsExtent);

			DrawDebugBox(World, BoundsOrigin, BoundsExtent, TargetOwner->GetActorQuat(), CandidateColor, false, -1.f, 0, bIsFocused ? 1.f : 0.5f);
		}

		DrawDebugCircle(
			World,
			FocusPoint,
			Profile.DetectionRadius,
			48,
			CandidateColor,
			false, -1.f, 0, 0.5f,
			FVector::ForwardVector,
			FVector::RightVector,
			false);

		DrawDebugPoint(World, ClosestPoint, 8.f, bIsFocused ? FColor::Yellow : FColor::Cyan, false, -1.f, 0);

		if (bIsFocused)
		{
			DrawDebugLine(World, AimOrigin, ClosestPoint, FColor::Yellow, false, -1.f, 0, 0.5f);
		}
	}

	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeLength = CVarInteractionDebugConeLength.GetValueOnGameThread();

		DrawDebugCone(
			World,
			AimOrigin,
			AimForward,
			ConeLength,
			ConeHalfAngleRad,
			ConeHalfAngleRad,
			24,
			FColor::Silver,
			false, -1.f, 0, 0.5f);
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

	FVector AimOrigin;
	FVector AimForward;
	InteractorPtr->GetAimPoint(AimOrigin, AimForward);

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

		const FInteractionFocusProfile& Profile = Candidate->GetFocusProfile();
		const FVector ClosestPoint = Candidate->GetClosestFocusPoint(AimOrigin);
		const float Distance = FVector::Dist(ClosestPoint, PawnLocation);
		const float Dot = FVector::DotProduct(AimForward, (ClosestPoint - AimOrigin).GetSafeNormal());

		const float AngularRange = 1.f - Profile.ConeCosine;
		const float AngularQuality = AngularRange > KINDA_SMALL_NUMBER
			? FMath::Clamp((Dot - Profile.ConeCosine) / AngularRange, 0.f, 1.f)
			: 1.f;

		const float DistanceQuality = Profile.DetectionRadius > KINDA_SMALL_NUMBER
			? FMath::Clamp(1.f - Distance / Profile.DetectionRadius, 0.f, 1.f)
			: 0.f;

		const bool bIsFocused = Candidate == FocusedTarget;

		float Score = Profile.AngularWeight * AngularQuality + Profile.DistanceWeight * DistanceQuality;
		if (bIsFocused)
		{
			Score += InteractorPtr->StickyFocusBonus;
		}

		const AActor* TargetOwner = Candidate->GetOwner();
		DrawLine(FString::Printf(TEXT("%s%s [%s]: Ang=%.2f Dist=%.2f Score=%.2f"),
			bIsFocused ? TEXT("* ") : TEXT("  "),
			TargetOwner ? *TargetOwner->GetName() : TEXT("?"),
			*Candidate->GetFocusProfileName().ToString(),
			AngularQuality, DistanceQuality, Score),
			bIsFocused ? HeaderColor : TextColor);
	}

	Y += LineHeight * 0.5f;
	DrawLine(TEXT("-- Prompts --"), HeaderColor);

	const TArray<FInteractionPrompt>& Prompts = InteractorPtr->GetPrompts();

	if (Prompts.IsEmpty())
	{
		DrawLine(TEXT("None"), TextColor);
	}
	else
	{
		for (const FInteractionPrompt& Prompt : Prompts)
		{
			DrawLine(FString::Printf(TEXT("  [%s] %s Enabled=%s"),
				*Prompt.InputTag.ToString(),
				*Prompt.ActionTag.ToString(),
				Prompt.bEnabled ? TEXT("true") : TEXT("false")),
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
