// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FVInteractionSubsystem.h"

#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"
#include "FVCore/Public/UI/FVInteractionActionInfo.h"
#include "Interfaces/FVActorWithTags.h"

void UFVInteractionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UFVInteractionSubsystem::Deinitialize()
{
	Target.Reset();
	
	Super::Deinitialize();
}

void UFVInteractionSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);
	
	if (IsValid(PlayerController.Get()))
	{
		PlayerController->OnPossessedPawnChanged.Clear();	
	}
	
	if (IsValid(NewPlayerController))
	{
		PlayerController = NewPlayerController;
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPawnChanged);
		
		OnPawnChanged(nullptr, NewPlayerController->GetPawn());
	}
}

UFVInteractionInstigatorComponent* UFVInteractionSubsystem::GetInstigator() const
{
	return Instigator.Get();
}

UFVInteractionTargetComponent* UFVInteractionSubsystem::GetFocusedTarget() const
{
	return Target.Get();
}

TArray<UFVInteractionAction*> UFVInteractionSubsystem::GetAvailableActions() const
{
	TArray<UFVInteractionAction*> AvailableActions;
	
	if (IsValid(Target.Get()))
	{
		AvailableActions  = Target.Get()->GetAvailableActions();
	}
	
	return AvailableActions;
}

bool UFVInteractionSubsystem::ActionRequirementsAreMet(const FGameplayTag& ActionTag) const
{
	if (!Target.Get())
	{
		return false;
	}
	
	FGameplayTagContainer ActorTags;
	
	if (IFVActorWithTags* InstigatorWithTags = Cast<IFVActorWithTags>(Instigator->GetOwner()))
	{
		ActorTags = InstigatorWithTags->GetAllTags();
	}
	
	for (const UFVInteractionAction* Action : Target->GetAvailableActions())
	{
		if (Action->ActionTag.MatchesTagExact(ActionTag) && Action->CheckRequirements(ActorTags))
		{
			return true;
		}
	}
	
	return false;
}

EFVInteractionResult UFVInteractionSubsystem::TryExecuteInteraction(const FGameplayTag& ActionTag)
{
	if (!Instigator->GetOwner() || !Target.Get())
	{
		return EFVInteractionResult::NoInteractable;
	}
	
	const EFVInteractionResult Result = Target->TryExecuteAction(ActionTag, Instigator->GetOwner());
	
	if (Result == EFVInteractionResult::Success)
	{
		OnFocusChanged.Broadcast(Target.Get());
	}

	return Result;
}

void UFVInteractionSubsystem::CancelCurrentInteraction() const
{
	if (UFVInteractionTargetComponent* CurrentTarget = GetFocusedTarget())
	{
		CurrentTarget->CancelActiveInteraction();
	}
}

TArray<FFVInteractionActionInfo> UFVInteractionSubsystem::GetAvailableActionsUIInfo() const
{
	TArray<UFVInteractionAction*> AvailableActions = GetAvailableActions();
	TArray<FFVInteractionActionInfo> ActionsInfo;
	
	if (IFVActorWithTags* InstigatorWithTags = Cast<IFVActorWithTags>(Instigator->GetOwner()))
	{
		FGameplayTagContainer& ActorTags = InstigatorWithTags->GetAllTags();
		
		for (const UFVInteractionAction* Action : AvailableActions)
		{
			ActionsInfo.Add(Action->CreateActionUIInfo(ActorTags));
		}
	}
	
	return ActionsInfo;
}

void UFVInteractionSubsystem::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (IsValid(NewPawn))
	{
		Instigator = NewPawn->GetComponentByClass<UFVInteractionInstigatorComponent>();
	}
}

void UFVInteractionSubsystem::UpdateFocus(UFVInteractionTargetComponent* NewTarget)
{
	if (Target.Get() == NewTarget)
	{
		return;
	}
	
	if (UFVInteractionTargetComponent* Previous = Target.Get())
	{
		Previous->SetFocused(false);
	}

	Target = NewTarget;
	
	if (NewTarget)
	{
		NewTarget->SetFocused(true);
	}
	
	OnFocusChanged.Broadcast(NewTarget);
}
