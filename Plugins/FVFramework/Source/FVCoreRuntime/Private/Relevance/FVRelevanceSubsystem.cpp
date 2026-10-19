#include "Relevance/FVRelevanceSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Relevance/FVRelevanceComponent.h"
#include "Relevance/FVRelevanceSettings.h"
#include "SignificanceManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRelevanceSubsystem)

UFVRelevanceSubsystem* UFVRelevanceSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFVRelevanceSubsystem>() : nullptr;
}

bool UFVRelevanceSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UFVRelevanceSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFVRelevanceSubsystem, STATGROUP_Tickables);
}

USignificanceManager* UFVRelevanceSubsystem::GetSignificanceManager() const
{
	return USignificanceManager::Get(GetWorld());
}

bool UFVRelevanceSubsystem::Register(UFVRelevanceComponent* Component)
{
	USignificanceManager* Manager = GetSignificanceManager();
	if (Manager == nullptr || !IsValid(Component))
	{
		return false;
	}

	// Significance is the negative distance, so the manager keeps the closest viewpoint and sorts closest first.
	// Band changes are only collected here and broadcast after the update, so listeners may spawn or destroy
	// relevance objects without touching the manager mid-update.
	Manager->RegisterObject(Component, Component->GetGroup().GetTagName(),
		[](USignificanceManager::FManagedObjectInfo* Info, const FTransform& Viewpoint)
		{
			return static_cast<const UFVRelevanceComponent*>(Info->GetObject())->ScoreViewpoint(Viewpoint);
		},
		USignificanceManager::EPostSignificanceType::Sequential,
		[this](USignificanceManager::FManagedObjectInfo* Info, float, const float Significance, bool)
		{
			UFVRelevanceComponent* Changed = static_cast<UFVRelevanceComponent*>(Info->GetObject());
			if (Changed->UpdateBand(-Significance))
			{
				ChangedComponents.Add(Changed);
			}
		});

	return true;
}

void UFVRelevanceSubsystem::Unregister(UFVRelevanceComponent* Component)
{
	if (USignificanceManager* Manager = GetSignificanceManager())
	{
		Manager->UnregisterObject(Component);
	}
}

void UFVRelevanceSubsystem::GetComponents(const FGameplayTag& Group, TArray<UFVRelevanceComponent*>& OutComponents) const
{
	if (const USignificanceManager* Manager = GetSignificanceManager())
	{
		for (const USignificanceManager::FManagedObjectInfo* Info : Manager->GetManagedObjects(Group.GetTagName()))
		{
			if (UFVRelevanceComponent* Component = Cast<UFVRelevanceComponent>(Info->GetObject()))
			{
				OutComponents.Add(Component);
			}
		}
	}
}

void UFVRelevanceSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Real time, so slow motion (scan mode) doesn't slow relevance down.
	const double Now = GetWorld()->GetRealTimeSeconds();
	if (Now - LastUpdateTime < UFVRelevanceSettings::Get().UpdateInterval)
	{
		return;
	}
	LastUpdateTime = Now;

	USignificanceManager* Manager = GetSignificanceManager();
	if (Manager == nullptr)
	{
		return;
	}

	GatherViewpoints();

	// Without a viewpoint the manager scores everything 0, which would read as distance 0; keep the last bands instead.
	if (Viewpoints.IsEmpty())
	{
		return;
	}

	Manager->Update(Viewpoints);

	TArray<TWeakObjectPtr<UFVRelevanceComponent>> ToBroadcast = MoveTemp(ChangedComponents);
	ChangedComponents.Reset();
	for (const TWeakObjectPtr<UFVRelevanceComponent>& Component : ToBroadcast)
	{
		UFVRelevanceComponent* Resolved = Component.Get();
		if (Resolved != nullptr && Resolved->bRegistered)
		{
			Resolved->BroadcastBandChange();
		}
	}
}

void UFVRelevanceSubsystem::GatherViewpoints()
{
	Viewpoints.Reset();

	const EFVRelevanceOrigin Origin = UFVRelevanceSettings::Get().Origin;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (PlayerController == nullptr || !PlayerController->IsLocalController())
		{
			continue;
		}

		FVector Location;
		FRotator Rotation;
		PlayerController->GetPlayerViewPoint(Location, Rotation);

		if (const APawn* Pawn = PlayerController->GetPawn(); Pawn != nullptr && Origin == EFVRelevanceOrigin::Pawn)
		{
			Location = Pawn->GetActorLocation();
		}

		Viewpoints.Emplace(Rotation, Location);
	}
}
