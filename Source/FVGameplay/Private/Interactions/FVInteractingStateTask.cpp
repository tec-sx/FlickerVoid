#include "Interactions/FVInteractingStateTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "FVGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interactions/FVInteractionOfferComponent.h"

namespace
{
	UAbilitySystemComponent* GetASC(AActor* Actor)
	{
		const IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(Actor);
		return Interface ? Interface->GetAbilitySystemComponent() : nullptr;
	}
}

EStateTreeRunStatus UFVInteractingStateTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	if (!InteractionOwner)
	{
		return EStateTreeRunStatus::Failed;
	}

	Offers = InteractionOwner->FindComponentByClass<UFVInteractionOfferComponent>();
	if (!Offers || !Offers->IsInteracting())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	ApplyInteractingState(true);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UFVInteractingStateTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	return (Offers && Offers->IsInteracting())
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void UFVInteractingStateTask::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	// Leaving this state for any reason must release the engaged target, so an
	// externally forced transition (death, combat) cannot strand a running tree.
	if (Offers && Offers->IsInteracting())
	{
		Offers->AbortEngagedInteraction(EFVInteractionCancelReason::Scripted);
	}

	ApplyInteractingState(false);
	Offers = nullptr;
}

void UFVInteractingStateTask::ApplyInteractingState(bool bInteracting)
{
	if (!InteractionOwner)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetASC(InteractionOwner))
	{
		ASC->SetLooseGameplayTagCount(FVGameplayTags::Status_Interacting, bInteracting ? 1 : 0);
	}

	if (bSuppressMovement)
	{
		if (const ACharacter* Character = Cast<ACharacter>(InteractionOwner))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				if (bInteracting)
				{
					Movement->StopMovementImmediately();
				}
				Movement->SetMovementMode(bInteracting ? MOVE_None : MOVE_Walking);
			}
		}
	}
}
