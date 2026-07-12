// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/FVSTT_GetRandomLocation.h"

#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "Actors/FVAICharacter.h"

EStateTreeRunStatus FFVSTT_GetRandomLocation::EnterState(FStateTreeExecutionContext& Context,
                                                         const FStateTreeTransitionResult& Transition) const
{
	FGetRandomLocationTaskInstanceData& Data = Context.GetInstanceData(*this);
	
	if (!Data.Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("FFVSTT_GetRandomLocation: Actor is null."));
		return EStateTreeRunStatus::Failed;
	}

	UWorld* World = Data.Actor->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("FFVSTT_GetRandomLocation: World is null."));
		return EStateTreeRunStatus::Failed;
	}

	const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys)
	{
		UE_LOG(LogTemp, Warning, TEXT("FFVSTT_GetRandomLocation: NavSys is null."));
		return EStateTreeRunStatus::Failed;
	}

	const FVector Origin = Data.Actor->GetActorLocation();
	FNavLocation NavLocation;
	
	if (NavSys->GetRandomReachablePointInRadius(Origin, Data.SearchRadius, NavLocation))
	{
		Data.RandomLocation = NavLocation.Location;
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PGAS_GetRandomLocation: Could not find random location."));
		return EStateTreeRunStatus::Failed;
	}
}