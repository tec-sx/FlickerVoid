#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Engine/World.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionRegistrySubsystem)

bool UFVInteractionRegistrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UFVInteractionRegistrySubsystem::Deinitialize()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BroadPhaseTimer);
	}

	InteractorRanges.Reset();
	Interactables.Reset();

	Super::Deinitialize();
}

void UFVInteractionRegistrySubsystem::Register(UFVInteractableComponent* Interactable)
{
	if (IsValid(Interactable))
	{
		Interactables.AddUnique(Interactable);
	}
}

void UFVInteractionRegistrySubsystem::Unregister(UFVInteractableComponent* Interactable)
{
	Interactables.RemoveSingleSwap(Interactable, EAllowShrinking::No);

	for (FInteractorRangeSet& RangeSet : InteractorRanges)
	{
		RangeSet.InRange.RemoveSingleSwap(Interactable, EAllowShrinking::No);
	}
}

void UFVInteractionRegistrySubsystem::RegisterInteractor(UFVInteractorComponent* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}

	const bool bAlreadyTracked = InteractorRanges.ContainsByPredicate(
		[Interactor](const FInteractorRangeSet& RangeSet) { return RangeSet.Interactor == Interactor; });

	if (bAlreadyTracked)
	{
		return;
	}

	FInteractorRangeSet& RangeSet = InteractorRanges.AddDefaulted_GetRef();
	RangeSet.Interactor = Interactor;

	StartBroadPhase();
}

void UFVInteractionRegistrySubsystem::UnregisterInteractor(UFVInteractorComponent* Interactor)
{
	const int32 Index = InteractorRanges.IndexOfByPredicate(
		[Interactor](const FInteractorRangeSet& RangeSet) { return RangeSet.Interactor == Interactor; });

	if (Index == INDEX_NONE)
	{
		return;
	}

	TArray<TObjectPtr<UFVInteractableComponent>> Orphaned = MoveTemp(InteractorRanges[Index].InRange);
	InteractorRanges.RemoveAtSwap(Index, EAllowShrinking::No);

	for (const TObjectPtr<UFVInteractableComponent>& Interactable : Orphaned)
	{
		if (IsValid(Interactable) && !IsInRange(Interactable))
		{
			Interactable->SetState(EFVInteractableState::Idle);
		}
	}

	if (InteractorRanges.IsEmpty())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(BroadPhaseTimer);
		}
	}
}

void UFVInteractionRegistrySubsystem::StartBroadPhase()
{
	UWorld* World = GetWorld();
	if (!World || World->GetTimerManager().IsTimerActive(BroadPhaseTimer))
	{
		return;
	}

	const float Interval = UFVInteractionSystemSettings::Get().InteractorDefaultSettings.BroadPhaseInterval;

	World->GetTimerManager().SetTimer(
		BroadPhaseTimer,
		FTimerDelegate::CreateUObject(this, &UFVInteractionRegistrySubsystem::RunBroadPhase),
		FMath::Max(Interval, 0.01f),
		true);

	RunBroadPhase();
}

void UFVInteractionRegistrySubsystem::RunBroadPhase()
{
	TArray<TObjectPtr<UFVInteractableComponent>> Entered;
	TArray<TObjectPtr<UFVInteractableComponent>> Left;

	for (int32 Index = InteractorRanges.Num() - 1; Index >= 0; --Index)
	{
		FInteractorRangeSet& RangeSet = InteractorRanges[Index];
		UFVInteractorComponent* Interactor = RangeSet.Interactor.Get();

		if (!IsValid(Interactor))
		{
			InteractorRanges.RemoveAtSwap(Index, EAllowShrinking::No);
			continue;
		}

		const FVector Origin = Interactor->GetDetectionOrigin();

		Entered.Reset();

		for (const TObjectPtr<UFVInteractableComponent>& Interactable : Interactables)
		{
			if (!IsValid(Interactable))
			{
				continue;
			}

			const float RadiusSq = FMath::Square(Interactable->GetDetectionRadius());

			if (FVector::DistSquared(Origin, Interactable->GetFocusPoint()) <= RadiusSq)
			{
				Entered.Add(Interactable);
			}
		}

		Left.Reset();

		for (const TObjectPtr<UFVInteractableComponent>& Previous : RangeSet.InRange)
		{
			if (!Entered.Contains(Previous))
			{
				Left.Add(Previous);
			}
		}

		const bool bHadAny = !RangeSet.InRange.IsEmpty();
		RangeSet.InRange = Entered;
		const bool bHasAny = !RangeSet.InRange.IsEmpty();

		for (const TObjectPtr<UFVInteractableComponent>& Interactable : Entered)
		{
			Interactable->SetState(EFVInteractableState::Awake);
		}

		for (const TObjectPtr<UFVInteractableComponent>& Interactable : Left)
		{
			if (IsValid(Interactable) && !IsInRange(Interactable))
			{
				Interactable->SetState(EFVInteractableState::Idle);
			}
		}

		if (bHadAny != bHasAny)
		{
			OnInRangeSetChanged.Broadcast(bHasAny);
		}
	}
}

bool UFVInteractionRegistrySubsystem::IsInRange(const UFVInteractableComponent* Interactable) const
{
	for (const FInteractorRangeSet& RangeSet : InteractorRanges)
	{
		if (RangeSet.InRange.Contains(Interactable))
		{
			return true;
		}
	}

	return false;
}

const TArray<TObjectPtr<UFVInteractableComponent>>& UFVInteractionRegistrySubsystem::GetInRangeSet(const UFVInteractorComponent* Interactor) const
{
	for (const FInteractorRangeSet& RangeSet : InteractorRanges)
	{
		if (RangeSet.Interactor == Interactor)
		{
			return RangeSet.InRange;
		}
	}

	static const TArray<TObjectPtr<UFVInteractableComponent>> Empty;
	return Empty;
}

void UFVInteractionRegistrySubsystem::QueryInRange(const FVector& Origin, float MaxRadius, TArray<UFVInteractableComponent*>& OutResults) const
{
	const float MaxRadiusSq = FMath::Square(MaxRadius);

	OutResults.Reset();
	OutResults.Reserve(Interactables.Num());

	for (const TObjectPtr<UFVInteractableComponent>& Interactable : Interactables)
	{
		if (FVector::DistSquared(Origin, Interactable->GetFocusPoint()) <= MaxRadiusSq)
		{
			OutResults.Add(Interactable);
		}
	}
}