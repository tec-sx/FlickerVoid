#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionInstigatorComponent)

static TAutoConsoleVariable CVarInteractionDebug(TEXT("FVCvar.Interaction.Debug"), false, TEXT("Debug the interaction system"));

UFVInteractionInstigatorComponent::UFVInteractionInstigatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFVInteractionInstigatorComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction Component has invalid owner."));
		return;
	}

	bIsInitialized = true;
	RefreshTickState();
}

void UFVInteractionInstigatorComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsInitialized)
	{
		return;
	}

	TimeSinceLastUpdate += DeltaTime;

	if (DetectionUpdateInterval <= 0.f || TimeSinceLastUpdate >= DetectionUpdateInterval)
	{
		TimeSinceLastUpdate = 0.f;
		DetectInteractables();
	}
}

void UFVInteractionInstigatorComponent::RegisterCandidate(UFVInteractionTargetComponent* Target)
{
	if (!Target)
	{
		return;
	}

	Candidates.AddUnique(Target);
	RefreshTickState();
}

void UFVInteractionInstigatorComponent::UnregisterCandidate(UFVInteractionTargetComponent* Target)
{
	if (!Target)
	{
		return;
	}

	Candidates.RemoveAll([Target](const TWeakObjectPtr<UFVInteractionTargetComponent>& Candidate)
		{
			return !Candidate.IsValid() || Candidate.Get() == Target;
		});

	if (FocusedTarget.Get() == Target)
	{
		FocusedTarget->SetFocused(false);
		FocusedTarget = nullptr;
		OnFocusChanged.Broadcast(nullptr);
	}

	RefreshTickState();
}

void UFVInteractionInstigatorComponent::RefreshTickState()
{
	SetComponentTickEnabled(bIsInitialized && Candidates.Num() > 0);
}

void UFVInteractionInstigatorComponent::DetectInteractables()
{
	UFVInteractionTargetComponent* NewTarget = FindBestTarget();

	if (FocusedTarget.Get() == NewTarget)
	{
		return;
	}

	if (UFVInteractionTargetComponent* Previous = FocusedTarget.Get())
	{
		Previous->SetFocused(false);
	}

	FocusedTarget = NewTarget;

	if (NewTarget)
	{
		NewTarget->SetFocused(true);
	}

	OnFocusChanged.Broadcast(NewTarget);
}

UFVInteractionTargetComponent* UFVInteractionInstigatorComponent::FindBestTarget() const
{
	FVector ViewLocation = Owner->GetActorLocation();
	FVector ViewForward = Owner->GetActorForwardVector();

	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
			ViewForward = ViewRotation.Vector();
		}
	}

	UFVInteractionTargetComponent* BestTarget = nullptr;
	float BestScore = -1.f;

	for (const TWeakObjectPtr<UFVInteractionTargetComponent>& CandidatePtr : Candidates)
	{
		UFVInteractionTargetComponent* Target = CandidatePtr.Get();
		if (!Target)
		{
			continue;
		}

		const FVector ProbeLocation = Target->GetAimProbeLocation();
		const FVector ToTarget = ProbeLocation - ViewLocation;
		const float Distance = ToTarget.Size();

		if (Distance > Target->GetFocusRadius())
		{
			continue;
		}

		const float Dot = FVector::DotProduct(ViewForward, ToTarget.GetSafeNormal());
		const FFVInteractionFocusProfile Profile = Target->GetFocusProfile();

		if (Dot < Profile.ConeCosine)
		{
			continue;
		}

		const float AngularRange = 1.f - Profile.ConeCosine;
		const float AngularQuality = AngularRange > KINDA_SMALL_NUMBER
			? FMath::Clamp((Dot - Profile.ConeCosine) / AngularRange, 0.f, 1.f)
			: 1.f;

		const float FocusRadius = Target->GetFocusRadius();
		const float DistanceQuality = FocusRadius > KINDA_SMALL_NUMBER
			? FMath::Clamp(1.f - Distance / FocusRadius, 0.f, 1.f)
			: 0.f;

		float Score = Profile.AngularWeight * AngularQuality + Profile.DistanceWeight * DistanceQuality;

		if (Target == FocusedTarget.Get())
		{
			Score += StickyFocusBonus;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}
