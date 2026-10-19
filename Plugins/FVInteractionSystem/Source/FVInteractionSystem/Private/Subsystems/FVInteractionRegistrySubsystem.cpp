#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#if !UE_BUILD_SHIPPING
#include "Subsystems/FVInteractionDebugSubsystem.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionRegistrySubsystem)

bool UFVInteractionRegistrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UFVInteractionRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if !UE_BUILD_SHIPPING
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		DebugSubsystem = GameInstance->GetSubsystem<UFVInteractionDebugSubsystem>();
	}
#endif
}

void UFVInteractionRegistrySubsystem::Deinitialize()
{
	UnregisterInteractor();
	InRangeInteractables.Reset();
	RegisteredInteractables.Reset();

	Super::Deinitialize();
}

const TArray<TObjectPtr<UFVInteractableComponent>>& UFVInteractionRegistrySubsystem::GetActiveInteractables() const
{
	static const TArray<TObjectPtr<UFVInteractableComponent>> None;
	return InteractorPtr.IsValid() ? InRangeInteractables : None;
}

void UFVInteractionRegistrySubsystem::Register(UFVInteractableComponent* Interactable)
{
	if (IsValid(Interactable))
	{
		RegisteredInteractables.AddUnique(Interactable);
	}
}

void UFVInteractionRegistrySubsystem::Unregister(UFVInteractableComponent* Interactable)
{
	InRangeInteractables.RemoveSingleSwap(Interactable, EAllowShrinking::No);
	RegisteredInteractables.RemoveSingleSwap(Interactable, EAllowShrinking::No);
}

void UFVInteractionRegistrySubsystem::SetInRange(UFVInteractableComponent* Interactable, const bool bInRange)
{
	if (!IsValid(Interactable))
	{
		return;
	}

	if (bInRange)
	{
		if (InRangeInteractables.Contains(Interactable))
		{
			return;
		}
		InRangeInteractables.Add(Interactable);

		if (InteractorPtr.IsValid())
		{
			Interactable->ActivateInteractions();
		}
	}
	else if (InRangeInteractables.RemoveSingleSwap(Interactable, EAllowShrinking::No) > 0 && InteractorPtr.IsValid())
	{
		Interactable->DeactivateInteractions();
	}
}

void UFVInteractionRegistrySubsystem::RegisterInteractor(UFVInteractorComponent* InInteractor)
{
	if (!IsValid(InInteractor))
	{
		return;
	}
	if (InInteractor == InteractorPtr)
	{
		return;
	}

	ensureMsgf(!InteractorPtr.IsValid(), TEXT("Only one player interactor can be registered per game."));

	InteractorPtr = InInteractor;
	InteractorPtr->EnableTracing();

	for (UFVInteractableComponent* Interactable : InRangeInteractables)
	{
		if (IsValid(Interactable))
		{
			Interactable->ActivateInteractions();
		}
	}

#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->Register(InteractorPtr);
	}
#endif
}

void UFVInteractionRegistrySubsystem::UnregisterInteractor()
{
	if (InteractorPtr.IsValid())
	{
		for (UFVInteractableComponent* Interactable : InRangeInteractables)
		{
			if (IsValid(Interactable))
			{
				Interactable->DeactivateInteractions();
			}
		}
	}

	InteractorPtr.Reset();

#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->Unregister();
	}
#endif
}
