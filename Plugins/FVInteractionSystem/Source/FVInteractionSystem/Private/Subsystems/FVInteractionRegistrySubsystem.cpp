#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Engine/World.h"
#include "FVInteractionSystemSettings.h"
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
	
	TickInterval = FMath::Max(UFVInteractionSystemSettings::Get().RegistrySettings.RefreshInterval, 0.01f);
	
#if !UE_BUILD_SHIPPING
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		DebugSubsystem = GameInstance->GetSubsystem<UFVInteractionDebugSubsystem>();
	}
#endif
}

void UFVInteractionRegistrySubsystem::Deinitialize()
{	
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
		TimeSinceLastTick -= TickInterval;
	}
}

TStatId UFVInteractionRegistrySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFVInteractionRegistrySubsystem, STATGROUP_Tickables);
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

	ensureMsgf(!InteractorPtr.IsValid(), TEXT("Only one player interactor can be registered per game."));
	
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
	for (const TObjectPtr<UFVInteractableComponent>& Interactable : ActiveInteractables)
	{
		if (IsValid(Interactable))
		{
			Interactable->DeactivateInteractions();
		}
	}

	ActiveInteractables.Reset();
	InteractorPtr.Reset();

#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->Unregister();
	}
#endif
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
	
	for (int32 Index = RegisteredInteractables.Num() - 1; Index >= 0; --Index)
	{
		UFVInteractableComponent* Interactable = RegisteredInteractables[Index];
		const AActor* InteractableActor = IsValid(Interactable) ? Interactable->GetOwner() : nullptr;

		if (!IsValid(InteractableActor))
		{
			ActiveInteractables.RemoveSingleSwap(Interactable, EAllowShrinking::No);
			RegisteredInteractables.RemoveAtSwap(Index, EAllowShrinking::No);
			continue;
		}

		const bool bIsInRange = FVector::DistSquared(Origin, InteractableActor->GetActorLocation()) <= RadiusSq;
		const bool bIsActive = ActiveInteractables.Contains(Interactable);

		if (bIsInRange && !bIsActive)
		{
			Interactable->ActivateInteractions();
			ActiveInteractables.Add(Interactable);
		}
		else if (!bIsInRange && bIsActive)
		{
			Interactable->DeactivateInteractions();
			ActiveInteractables.RemoveSingleSwap(Interactable, EAllowShrinking::No);
		}
	}
	
#if !UE_BUILD_SHIPPING
	if (const UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->VisualizeRange(GetWorld(), Interactor, Settings.DefaultActivationRadius, ActiveInteractables, TickInterval);
	}
#endif
}
