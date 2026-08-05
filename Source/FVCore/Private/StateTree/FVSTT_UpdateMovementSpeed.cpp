// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/FVSTT_UpdateMovementSpeed.h"

#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSTT_UpdateMovementSpeed)

EStateTreeRunStatus UFVSTT_UpdateMovementSpeed::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("FVSTT_GetRandomLocation: Actor is null."));
		
		return EStateTreeRunStatus::Failed;
	}
	
	if (UCharacterMovementComponent* MoveComp = Actor->GetComponentByClass<UCharacterMovementComponent>())
	{
		MoveComp->MaxWalkSpeed = TargetMaxWalkSpeed;
		
		return EStateTreeRunStatus::Succeeded;
	}

	UE_LOG(LogTemp, Warning, TEXT("FVSTT_UpdateMovementSpeed: Character or MovementComponent not valid"));
	
	return EStateTreeRunStatus::Failed;
}

#if WITH_EDITOR
FText UFVSTT_UpdateMovementSpeed::GetDescription(
	const FGuid& ID, 
	FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, 
	EStateTreeNodeFormatting Formatting) const
{
	return FText::Format(
	NSLOCTEXT("FlickerVoid", "UpdateMovementSpeed", "Set Actor Max Walk Speed to {0}"),
		FText::AsNumber(TargetMaxWalkSpeed));
}
#endif