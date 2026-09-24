#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Engine/World.h"
#include "FVInteractionSystemSettings.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems//FVInteractionDebugSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionRegistrySubsystem)

bool UFVInteractionRegistrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UFVInteractionRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	TickInterval = FMath::Max(UFVInteractionSystemSettings::Get().RegistrySettings.RefreshInterval, 0.01f);
	
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		DebugSubsystem = GameInstance->GetSubsystem<UFVInteractionDebugSubsystem>();
	}
}

void UFVInteractionRegistrySubsystem::Deinitialize()
{
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->Unregister();
	}
#endif
	
	ActiveInteractables.Reset();
	RegisteredInteractables.Reset();

	Super::Deinitialize();
}

void UFVInteractionRegistrySubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TimeSinceLastTick += DeltaTime;
	
	if (TimeSinceLastTick >= TickInterval)
	{
		Update();
		TimeSinceLastTick = 0.f;
	}
}

TStatId UFVInteractionRegistrySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULoadingScreenManager, STATGROUP_Tickables);
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
	ActiveInteractables.RemoveSingleSwap(Interactable, EAllowShrinking::No);
	RegisteredInteractables.RemoveSingleSwap(Interactable, EAllowShrinking::No);
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
	
	InteractorPtr = InInteractor;
	InteractorPtr->EnableTracing();
	
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->Register(InteractorPtr);
	}
#endif
}

void UFVInteractionRegistrySubsystem::UnregisterInteractor()
{
	TArray<TObjectPtr<UFVInteractableComponent>> Orphaned = MoveTemp(ActiveInteractables);

	for (const TObjectPtr<UFVInteractableComponent>& Interactable : Orphaned)
	{
		if (IsValid(Interactable) && !IsActive(Interactable))
		{
			Interactable->SetState(EFVInteractableState::Idle);
		}
	}
}

bool UFVInteractionRegistrySubsystem::IsActive(const UFVInteractableComponent* Interactable)
{
	if (ActiveInteractables.Contains(Interactable))
	{
		return true;
	}
	
	return false;
}

void UFVInteractionRegistrySubsystem::Update()
{
	UFVInteractorComponent* Interactor = InteractorPtr.Get();
	
	if (!IsValid(Interactor))
	{
		ActiveInteractables.Reset();
		return;
	}
	
	const FVector Origin = Interactor->GetOwner() ? Interactor->GetOwner()->GetActorLocation() : FVector::ZeroVector;
		
	const FFVInteractionRegistrySettings& Settings = UFVInteractionSystemSettings::Get().RegistrySettings;
	const float RadiusSq = FMath::Square(Settings.DefaultActivationRadius);
	
	for (const TObjectPtr<UFVInteractableComponent>& Interactable : RegisteredInteractables)
	{
		if (!IsValid(Interactable))
			continue;	
		
		const AActor* InteractableActor = Interactable->GetOwner();
		
		if (!IsValid(InteractableActor))
			continue;
		
		const bool bIsInRange = FVector::DistSquared(Origin, InteractableActor->GetActorLocation()) <= RadiusSq;
		const bool bIsActive = ActiveInteractables.Contains(Interactable);
			
		if (bIsInRange)
		{
			if (!bIsActive)
			{
				Interactable->ActivateInteractions();
				ActiveInteractables.Add(Interactable);
			}
		}
		else
		{
			if (bIsActive)
			{
				Interactable->DeactivateInteractions();
				ActiveInteractables.Remove(Interactable);
			}
		}
	}
	
#if !UE_BUILD_SHIPPING
	if (const UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->VisualizeRange(GetWorld(), Interactor, Settings.DefaultActivationRadius, ActiveInteractables, TickInterval);
	}
#endif
}
