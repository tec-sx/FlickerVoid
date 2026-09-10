#include "Subsystems/InteractionRegistrySubsystem.h"
#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "Engine/World.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionRegistrySubsystem)

bool UInteractionRegistrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UInteractionRegistrySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BroadPhaseTimer);
	}

	InteractorRanges.Reset();
	Interactables.Reset();

	Super::Deinitialize();
}

void UInteractionRegistrySubsystem::Register(UInteractableComponent* Interactable)
{
	if (IsValid(Interactable))
	{
		Interactables.AddUnique(Interactable);
	}
}

void UInteractionRegistrySubsystem::Unregister(UInteractableComponent* Interactable)
{
	Interactables.RemoveSingleSwap(Interactable, EAllowShrinking::No);

	for (FInteractorRangeSet& RangeSet : InteractorRanges)
	{
		RangeSet.InRange.RemoveSingleSwap(Interactable, EAllowShrinking::No);
	}
}

void UInteractionRegistrySubsystem::RegisterInteractor(UInteractorComponent* Interactor)
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

void UInteractionRegistrySubsystem::UnregisterInteractor(UInteractorComponent* Interactor)
{
	const int32 Index = InteractorRanges.IndexOfByPredicate(
		[Interactor](const FInteractorRangeSet& RangeSet) { return RangeSet.Interactor == Interactor; });

	if (Index == INDEX_NONE)
	{
		return;
	}

	TArray<TObjectPtr<UInteractableComponent>> Orphaned = MoveTemp(InteractorRanges[Index].InRange);
	InteractorRanges.RemoveAtSwap(Index, EAllowShrinking::No);

	for (const TObjectPtr<UInteractableComponent>& Interactable : Orphaned)
	{
		if (IsValid(Interactable) && !IsInRangeOfAnyInteractor(Interactable))
		{
			ApplyRangeState(Interactable, false);
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

void UInteractionRegistrySubsystem::StartBroadPhase()
{
	UWorld* World = GetWorld();
	if (!World || World->GetTimerManager().IsTimerActive(BroadPhaseTimer))
	{
		return;
	}

	const float Interval = UFVInteractionSystemSettings::Get().InteractorDefaultSettings.BroadPhaseInterval;

	World->GetTimerManager().SetTimer(
		BroadPhaseTimer,
		FTimerDelegate::CreateUObject(this, &UInteractionRegistrySubsystem::RunBroadPhase),
		FMath::Max(Interval, 0.01f),
		true);

	RunBroadPhase();
}

void UInteractionRegistrySubsystem::RunBroadPhase()
{
	TArray<TObjectPtr<UInteractableComponent>> Entered;
	TArray<TObjectPtr<UInteractableComponent>> Left;

	for (int32 Index = InteractorRanges.Num() - 1; Index >= 0; --Index)
	{
		FInteractorRangeSet& RangeSet = InteractorRanges[Index];
		UInteractorComponent* Interactor = RangeSet.Interactor.Get();

		if (!IsValid(Interactor))
		{
			InteractorRanges.RemoveAtSwap(Index, EAllowShrinking::No);
			continue;
		}

		const FVector Origin = Interactor->GetDetectionOrigin();

		Entered.Reset();

		for (const TObjectPtr<UInteractableComponent>& Interactable : Interactables)
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

		for (const TObjectPtr<UInteractableComponent>& Previous : RangeSet.InRange)
		{
			if (!Entered.Contains(Previous))
			{
				Left.Add(Previous);
			}
		}

		const bool bHadAny = !RangeSet.InRange.IsEmpty();
		RangeSet.InRange = Entered;
		const bool bHasAny = !RangeSet.InRange.IsEmpty();

		for (const TObjectPtr<UInteractableComponent>& Interactable : Entered)
		{
			ApplyRangeState(Interactable, true);
		}

		for (const TObjectPtr<UInteractableComponent>& Interactable : Left)
		{
			if (IsValid(Interactable) && !IsInRangeOfAnyInteractor(Interactable))
			{
				ApplyRangeState(Interactable, false);
			}
		}

		if (bHadAny != bHasAny)
		{
			OnInRangeSetChanged.Broadcast(bHasAny);
		}
	}
}

bool UInteractionRegistrySubsystem::IsInRangeOfAnyInteractor(const UInteractableComponent* Interactable) const
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

void UInteractionRegistrySubsystem::ApplyRangeState(UInteractableComponent* Interactable, bool bInRange) const
{
	const EInteractableState Desired = bInRange ? EInteractableState::Awake : EInteractableState::Idle;

	if (Interactable->GetState() == Desired)
	{
		return;
	}

	Interactable->TrySetState(Desired);
}

const TArray<TObjectPtr<UInteractableComponent>>& UInteractionRegistrySubsystem::GetInRangeSet(const UInteractorComponent* Interactor) const
{
	for (const FInteractorRangeSet& RangeSet : InteractorRanges)
	{
		if (RangeSet.Interactor == Interactor)
		{
			return RangeSet.InRange;
		}
	}

	static const TArray<TObjectPtr<UInteractableComponent>> Empty;
	return Empty;
}

void UInteractionRegistrySubsystem::QueryInRange(const FVector& Origin, float MaxRadius, TArray<UInteractableComponent*>& OutResults) const
{
	const float MaxRadiusSq = FMath::Square(MaxRadius);

	OutResults.Reset();
	OutResults.Reserve(Interactables.Num());

	for (const TObjectPtr<UInteractableComponent>& Interactable : Interactables)
	{
		if (FVector::DistSquared(Origin, Interactable->GetFocusPoint()) <= MaxRadiusSq)
		{
			OutResults.Add(Interactable);
		}
	}
}