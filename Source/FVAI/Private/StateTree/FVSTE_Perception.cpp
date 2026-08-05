// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/FVSTE_Perception.h"

#include "FVAICharacterController.h"
#include "FVAITags.h"
#include "FVStateTreeAIComponent.h"
#include "StateTreeExecutionContext.h"
#include "Actors/FVAICharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSTE_Perception)

void UFVSTE_Perception::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);
	
	AFVAICharacterController* aiController = Cast<AFVAICharacterController>(Context.GetOwner());
	
	if (!aiController)
	{
		UE_LOG(LogTemp, Error, TEXT("UFVSTE_Perception: StateTree owner is not an AFVEnemyAIController!"));
		return;
	}
	AIController = aiController;

	// Bind to controller's delegates (avoid duplicate binds)
	AIController->OnSightStimulusDetected.AddDynamic(this, &UFVSTE_Perception::HandleSightStimulus);
	AIController->OnSightStimulusForgotten.AddDynamic(this, &UFVSTE_Perception::HandleSightStimulusForgotten);
	AIController->OnHearingStimulusDetected.AddDynamic(this, &UFVSTE_Perception::HandleHearingStimulus);
	AIController->OnHearingStimulusForgotten.AddDynamic(this, &UFVSTE_Perception::HandleHearingStimulusForgotten);
	AIController->OnDamageStimulusDetected.AddDynamic(this, &UFVSTE_Perception::HandleDamageStimulus);
	
	AIController->CurrentStateTreeState = EFVStateTreeEvent::Unknown;
	
	APawn* OwnerPawn = AIController->GetPawn();
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UFVSTE_Perception: AIController has no Pawn!"));
		return;
	}

	// Set our context character
	OwnerCharacter = Cast<AFVAICharacter>(OwnerPawn);
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("UFVSTE_Perception: Pawn is not an AFVAICharacter! Class is: %s"), *OwnerPawn->GetClass()->GetName());
		return;
	}
	
	StateTreeComp = AIController->FindComponentByClass<UFVStateTreeAIComponent>();
	if (!StateTreeComp)
	{
		UE_LOG(LogTemp, Error, TEXT("UFVSTE_Perception: No UStateTreeAIComponent found on AIController!"));
	}
}

void UFVSTE_Perception::TreeStop(FStateTreeExecutionContext& Context)
{
	Super::TreeStop(Context);

	if (AIController.IsValid())
	{
		AIController->OnSightStimulusDetected.RemoveDynamic(this, &UFVSTE_Perception::HandleSightStimulus);
		AIController->OnSightStimulusForgotten.RemoveDynamic(this, &UFVSTE_Perception::HandleSightStimulusForgotten);
		AIController->OnHearingStimulusDetected.RemoveDynamic(this, &UFVSTE_Perception::HandleHearingStimulus);
		AIController->OnHearingStimulusForgotten.RemoveDynamic(this, &UFVSTE_Perception::HandleHearingStimulusForgotten);
		AIController->OnDamageStimulusDetected.RemoveDynamic(this, &UFVSTE_Perception::HandleDamageStimulus);

		AIController->CurrentStateTreeState = EFVStateTreeEvent::Unknown;
	}
	
	StateTreeComp = nullptr;
	AcquiredTarget = nullptr;
}

void UFVSTE_Perception::HandleSightStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (AIController->CurrentStateTreeState != EFVStateTreeEvent::Notice)
	{
		AcquiredTarget = Actor;
		if (AIController.IsValid())
		{
			AIController->AcquiredTarget = Actor;
		}
		
		OnSightStimulus(Actor, Stimulus);

		SendEvent(EFVStateTreeEvent::Notice);
		AIController->CurrentStateTreeState = EFVStateTreeEvent::Notice;
	}
}

void UFVSTE_Perception::HandleSightStimulusForgotten(AActor* Actor)
{
	UE_LOG(LogTemp, Log, TEXT("UFVSTE_Perception: Sight stimulus forgotten for actor: %s"), *GetNameSafe(Actor));
	AcquiredTarget = nullptr;
	if (AIController.IsValid())
	{
		AIController->AcquiredTarget = nullptr;
	}
	AIController->CurrentStateTreeState = EFVStateTreeEvent::Unknown;
	OnSightStimulusForgotten(Actor);
}

void UFVSTE_Perception::HandleHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (AIController->CurrentStateTreeState != EFVStateTreeEvent::Act)
	{
		AcquiredTarget = Actor;
		if (AIController.IsValid())
		{
			AIController->AcquiredTarget = Actor;
		}
		
		OnHearingStimulus(Actor, Stimulus);

		SendEvent(EFVStateTreeEvent::Act);
		AIController->CurrentStateTreeState = EFVStateTreeEvent::Act;
	}
}

void UFVSTE_Perception::HandleHearingStimulusForgotten(AActor* Actor)
{
	UE_LOG(LogTemp, Log, TEXT("UFVSTE_Perception: Hearing stimulus forgotten for actor: %s"), *GetNameSafe(Actor));
	AcquiredTarget = nullptr;
	if (AIController.IsValid())
	{
		AIController->AcquiredTarget = nullptr;
	}
	AIController->CurrentStateTreeState = EFVStateTreeEvent::Unknown;
	OnHearingStimulusForgotten(Actor);
}

void UFVSTE_Perception::HandleDamageStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (AIController->CurrentStateTreeState != EFVStateTreeEvent::Act)
	{
		AcquiredTarget = Actor;
		if (AIController.IsValid())
		{
			AIController->AcquiredTarget = Actor;
		}
		
		OnDamageStimulus(Actor, Stimulus);

		SendEvent(EFVStateTreeEvent::Act);
		AIController->CurrentStateTreeState = EFVStateTreeEvent::Act;
	}
}

void UFVSTE_Perception::SendEvent(const EFVStateTreeEvent InEvent)
{
	FGameplayTag EventTag;
	switch (InEvent)
	{
	case EFVStateTreeEvent::Idle:
		EventTag = FVAITags::AI_Event_Idle;
		LastEvent = EFVStateTreeEvent::Idle;
		break;
	case EFVStateTreeEvent::Patrol:
		EventTag = FVAITags::AI_Event_Patrol;
		LastEvent = EFVStateTreeEvent::Patrol;
		break;
	case EFVStateTreeEvent::Notice:
		EventTag = FVAITags::AI_Event_Notice;
		LastEvent = EFVStateTreeEvent::Idle;
		break;
	case EFVStateTreeEvent::Focus:
		EventTag = FVAITags::AI_Event_Focus;
		LastEvent = EFVStateTreeEvent::Notice;
		break;
	case EFVStateTreeEvent::Act:
		EventTag = FVAITags::AI_Event_Act;
		LastEvent = EFVStateTreeEvent::Focus;
		break;
	default:
		return; // Ignore unknown events
	}
	
	if (AIController.IsValid() && StateTreeComp)
	{
		const FStateTreeEvent Event(EventTag);
		StateTreeComp->SendStateTreeEvent(Event);
	}
}
