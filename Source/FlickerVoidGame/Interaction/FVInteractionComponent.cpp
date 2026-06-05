// Fill out your copyright notice in the Description page of Project Settings.

#include "FVInteractionComponent.h"
#include "Interfaces/FVInteractableInterface.h"
#include "FVInteractionActionData.h"
#include "FVAbilitySystemComponent.h"
#include "Abilities/FVGameplayAbility.h"
#include "Player/FVPlayerState.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionComponent)

/* 
- Remove Item Interaction as a gameplay ability.
- Create default input actions and bindings for interaction inputs
- Call interaction component functions directly from input bindings instead of through gameplay abilities
- Interaction 
*/


namespace FVCVars
{
	static TAutoConsoleVariable<bool> DebugDrawInteractionTarget(
		TEXT("FVoid.Interaction.Debug.DrawInteractionTarget"),
		false,
		TEXT("Whether to draw debug spheres at interaction targets. Only applies in non-shipping builds."),
		ECVF_Default);
}

UFVInteractionComponent::UFVInteractionComponent()
	: InteractionDistance(300.0f)
	, TraceRadius(20.0f)
	, TraceChannel(ECC_Visibility)
	, TraceFrequency(10.0f)
	, HoldTimeAccumulator(0.0f)
	, TraceCooldown(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame
}

void UFVInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFVInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update hold progress
	if (CurrentHoldAction)
	{
		HoldTimeAccumulator += DeltaTime;
		
		if (HoldTimeAccumulator >= CurrentHoldAction->HoldDuration)
		{
			// Hold completed - execute action
			ExecuteAction(CurrentHoldAction);
			StopHoldingAction();
		}
	}

	// Perform interaction trace
	TraceCooldown -= DeltaTime;
	if (TraceCooldown <= 0.0f)
	{
		PerformInteractionTrace();
		TraceCooldown = 1.0f / TraceFrequency;
	}
}

void UFVInteractionComponent::PerformInteractionTrace()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// Get camera for trace start/direction
	UCameraComponent* Camera = OwnerActor->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	FVector Start = Camera->GetComponentLocation();
	FVector ForwardVector = Camera->GetForwardVector();
	FVector End = Start + (ForwardVector * InteractionDistance);
	FCollisionShape CollisionSphere = FCollisionShape::MakeSphere(TraceRadius);
	AActor* FocusedActor = nullptr;

#if !UE_BUILD_SHIPPING
	FColor DebugColor = FColor::Red;
#endif
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		CollisionSphere,
		QueryParams
	);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		// Check if hit actor implements interactable interface
		if (HitActor && HitActor->Implements<UFVInteractableInterface>())
		{
			IFVInteractableInterface* Interactable = Cast<IFVInteractableInterface>(HitActor);
			bool bCanInteract = false; /*Interactable->CanInteract_Implementation(OwnerActor);*/

			if (bCanInteract)
			{
				FocusedActor = HitActor;
#if !UE_BUILD_SHIPPING
				DebugColor = FColor::Green;
#endif
			}

#if !UE_BUILD_SHIPPING
			if (FVCVars::DebugDrawInteractionTarget.GetValueOnAnyThread())
			{
				DrawDebugSphere(
					GetWorld(), 
					HitActor->GetActorLocation(),
					CollisionSphere.GetSphereRadius(), 
					12, 
					DebugColor, 
					false, 
					1.0f / TraceFrequency);
			}
		}
#endif
	}

	UpdateFocusedActor(FocusedActor);
}

void UFVInteractionComponent::UpdateFocusedActor(AActor* NewActor)
{
	if (CurrentFocusedActor == NewActor)
	{
		return;
	}

	// Handle focus lost
	if (CurrentFocusedActor)
	{
		if (CurrentFocusedActor->Implements<UFVInteractableInterface>())
		{
			//IFVInteractableInterface::Execute_OnInteractionFocusLost(CurrentFocusedActor, GetOwner());
		}
		OnFocusLost.Broadcast(CurrentFocusedActor);
	}

	CurrentFocusedActor = NewActor;
	CurrentActions.Empty();

	// Handle focus gained
	if (CurrentFocusedActor)
	{
		if (CurrentFocusedActor->Implements<UFVInteractableInterface>())
		{
			/*IFVInteractableInterface::Execute_OnInteractionFocusGained(CurrentFocusedActor, GetOwner());*/
			UpdateAvailableActions();
		}

		OnFocusGained.Broadcast(CurrentFocusedActor);
	}
}

void UFVInteractionComponent::UpdateAvailableActions()
{
	CurrentActions.Empty();

	if (!CurrentFocusedActor || !CurrentFocusedActor->Implements<UFVInteractableInterface>())
	{
		OnActionsChanged.Broadcast(nullptr, CurrentActions);
		return;
	}

	// Get actions from interactable
	TArray<UFVInteractionActionData*> AvailableActions = TArray<UFVInteractionActionData*>(); /*
		IFVInteractableInterface::Execute_GetInteractionActions(CurrentFocusedActor, GetOwner()); */
	
	// Get ASC for tag checking
	UFVAbilitySystemComponent* ASC = nullptr;
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AFVPlayerState* PS = Pawn->GetPlayerState<AFVPlayerState>())
		{
			ASC = PS->GetFVAbilitySystemComponent();
		}
	}

	for (UFVInteractionActionData* Action : AvailableActions)
	{
		if (!Action)
		{
			continue;
		}

		// Check required/blocked tags if ASC is available
		bool bAllowed = true;
		if (ASC)
		{
			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);

			if (Action->RequiredActorTags.Num() > 0 && !OwnedTags.HasAll(Action->RequiredActorTags))
			{
				bAllowed = false;
			}

			if (Action->BlockedByTags.Num() > 0 && OwnedTags.HasAny(Action->BlockedByTags))
			{
				bAllowed = false;
			}
		}

		if (bAllowed)
		{
			CurrentActions.Add(Action);
		}
	}

	// Sort by priority (descending)
	CurrentActions.Sort([](const UFVInteractionActionData& A, const UFVInteractionActionData& B)
	{
		return A.DisplayPriority > B.DisplayPriority;
	});

	// Limit to 4 actions max
	if (CurrentActions.Num() > 4)
	{
		CurrentActions.SetNum(4);
	}

	OnActionsChanged.Broadcast(CurrentFocusedActor, CurrentActions);
}

bool UFVInteractionComponent::ExecuteAction(UFVInteractionActionData* Action)
{
	if (!Action || !CurrentFocusedActor)
	{
		return false;
	}

	// Get ASC to activate ability
	UFVAbilitySystemComponent* ASC = nullptr;
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AFVPlayerState* PS = Pawn->GetPlayerState<AFVPlayerState>())
		{
			ASC = PS->GetFVAbilitySystemComponent();
		}
	}

	if (!ASC)
	{
		return false;
	}

	//// Find the ability spec for the action's ability class
	//FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(Action->AbilityToActivate);
	//if (!Spec)
	//{
	//	return false;
	//}

	//// Try to activate the ability with event data containing the interaction target
	//FGameplayEventData EventData;
	//EventData.Target = CurrentFocusedActor;
	//EventData.Instigator = GetOwner();
	//EventData.EventTag = Action->ActionTag;

	//return ASC->TriggerAbilityFromGameplayEvent(
	//	Spec->Handle,
	//	ASC->AbilityActorInfo.Get(),
	//	Action->ActionTag,
	//	&EventData,
	//	*ASC
	//);

	// TODO: For now just return true to indicate the action was "executed". Replace with actual ability activation logic once we have abilities set up.
	return false;
}

void UFVInteractionComponent::StartHoldingAction(UFVInteractionActionData* Action)
{
	if (!Action || Action->InputType != EFVInteractionInputType::Hold)
	{
		return;
	}

	CurrentHoldAction = Action;
	HoldTimeAccumulator = 0.0f;
}

void UFVInteractionComponent::StopHoldingAction()
{
	CurrentHoldAction = nullptr;
	HoldTimeAccumulator = 0.0f;
}

float UFVInteractionComponent::GetHoldProgress() const
{
	if (!CurrentHoldAction)
	{
		return 0.0f;
	}

	return FMath::Clamp(HoldTimeAccumulator / CurrentHoldAction->HoldDuration, 0.0f, 1.0f);
}
