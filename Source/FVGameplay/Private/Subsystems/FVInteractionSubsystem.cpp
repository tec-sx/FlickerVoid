// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FVInteractionSubsystem.h"

#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"
#include "FVCore/Public/UI/FVInteractionActionInfo.h"

void UFVInteractionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UFVInteractionSubsystem::Deinitialize()
{
	FocusedTarget.Reset();
	
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
	}
}

UFVInteractionInstigatorComponent* UFVInteractionSubsystem::GetInstigator() const
{
	return Instigator.Get();
}

UFVInteractionTargetComponent* UFVInteractionSubsystem::GetFocusedTarget() const
{
	return FocusedTarget.Get();
}

TArray<UFVInteractionAction*> UFVInteractionSubsystem::GetAvailableActions() const
{
	TArray<UFVInteractionAction*> AvailableActions;
	
	if (IsValid(FocusedTarget.Get()))
	{
		AvailableActions  = FocusedTarget.Get()->GetAvailableActions();
	}
	
	return AvailableActions;
}

TArray<FFVInteractionActionInfo> UFVInteractionSubsystem::GetAvailableActionsUIInfo() const
{
	TArray<UFVInteractionAction*> AvailableActions = GetAvailableActions();
	TArray<FFVInteractionActionInfo> ActionsInfo;
	
	for (const UFVInteractionAction* Action : AvailableActions)
	{
		ActionsInfo.Add(Action->CreateActionUIInfo(Instigator->GetTags()));
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
	if (FocusedTarget.Get() == NewTarget)
	{
		return;
	}
	
	if (UFVInteractionTargetComponent* Previous = FocusedTarget.Get())
	{
		Previous->SetFocused(false);
	}

	FocusedTarget = NewTarget;
	
	if (NewTarget)
	{
		NewTarget->SetFocused(true);
	}
	
	OnFocusChanged.Broadcast(NewTarget);
}
