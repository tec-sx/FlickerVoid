#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractorComponent)

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<APawn>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction Component has invalid pawn owner."));
		return;
	}

	bIsInitialized = true;
	SetComponentTickEnabled(bIsInitialized && Candidates.Num() > 0);
}


void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsInitialized)
	{
		TimeSinceLastUpdate += DeltaTime;

		if (DetectionUpdateInterval <= 0.f || TimeSinceLastUpdate >= DetectionUpdateInterval)
		{
			TimeSinceLastUpdate = 0.f;
			DetectInteractables();
		}
	}
}

void UInteractorComponent::RegisterCandidate(UInteractableComponent* Target)
{
	if (Target)
	{
		Candidates.AddUnique(Target);
		SetComponentTickEnabled(true);
	}
}

void UInteractorComponent::UnregisterCandidate(UInteractableComponent* Target)
{
	if (Target)
	{
		Candidates.RemoveAll([Target](const TWeakObjectPtr<UInteractableComponent>& Candidate)
			{
				return !Candidate.IsValid() || Candidate.Get() == Target;
			});

		if (FocusedTarget.Get() == Target)
		{
			FocusedTarget->SetFocused(false);
			FocusedTarget = nullptr;
			OnFocusChanged.Broadcast(nullptr);
		}

		SetComponentTickEnabled(bIsInitialized && Candidates.Num() > 0);
	}
}

void UInteractorComponent::DetectInteractables()
{
	FVector ViewLocation = Owner->GetActorLocation();
	FVector ViewForward = Owner->GetActorForwardVector();

	if (const APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
	{
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
		ViewForward = ViewRotation.Vector();
	}

	UInteractableComponent* BestCandidate = nullptr;
	float BestScore = -1.f;

	for (const TWeakObjectPtr<UInteractableComponent>& CandidatePtr : Candidates)
	{
		UInteractableComponent* Candidate = CandidatePtr.Get();

		if (Candidate)
		{
			const FVector ProbeLocation = Candidate->GetAimProbeLocation();
			const FInteractionFocusProfile Profile = Candidate->GetFocusProfile();

			const FVector ToTarget = ProbeLocation - ViewLocation;
			const float Dot = FVector::DotProduct(ViewForward, ToTarget.GetSafeNormal());

			if (Dot < Profile.ConeCosine)
			{
				continue;
			}

			const float AngularRange = 1.f - Profile.ConeCosine;
			const float AngularQuality = AngularRange > KINDA_SMALL_NUMBER
				? FMath::Clamp((Dot - Profile.ConeCosine) / AngularRange, 0.f, 1.f)
				: 1.f;

			const float FocusRadius = Candidate->GetScaledSphereRadius();
			const float Distance = ToTarget.Size();
			const float DistanceQuality = FocusRadius > KINDA_SMALL_NUMBER
				? FMath::Clamp(1.f - Distance / FocusRadius, 0.f, 1.f)
				: 0.f;

			float Score = Profile.AngularWeight * AngularQuality + Profile.DistanceWeight * DistanceQuality;

			if (Candidate == FocusedTarget.Get())
			{
				Score += StickyFocusBonus;
			}

			if (Score > BestScore)
			{
				BestScore = Score;
				BestCandidate = Candidate;
			}
		}
	}
}