#include "Spawn/FVSpawner.h"

#include "Components/BillboardComponent.h"
#include "Engine/World.h"
#include "Facts/FVFactDatabase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "FVSpawner"

#if WITH_EDITOR
EDataValidationResult UFVSpawnDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (!ActorClass.IsNull())
	{
		return Result;
	}
	Context.AddError(LOCTEXT("NoClass", "Spawn definition has no ActorClass."));
	return EDataValidationResult::Invalid;
}
#endif

AFVSpawner::AFVSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
#if WITH_EDITORONLY_DATA
	CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"))->SetupAttachment(RootComponent);
#endif
}

void AFVSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		FactHandle = Facts->OnFactChangedNative().AddUObject(this, &AFVSpawner::HandleFactChanged);
	}
	Evaluate();
}

void AFVSpawner::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->OnFactChangedNative().Remove(FactHandle);
	}
	Super::EndPlay(Reason);
}

void AFVSpawner::HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
	Evaluate();
}

FFVConditionContext AFVSpawner::MakeContext() const
{
	FFVConditionContext Context;
	Context.WorldContext = const_cast<AFVSpawner*>(this);
	Context.Instigator = UGameplayStatics::GetPlayerPawn(this, 0);
	Context.Target = SpawnedActor;
	return Context;
}

void AFVSpawner::Evaluate()
{
	if (!Definition)
	{
		return;
	}
	const FFVConditionContext Context = MakeContext();
	if (SpawnedActor)
	{
		if (!Definition->DespawnWhen.IsEmpty() && Definition->DespawnWhen.Evaluate(Context))
		{
			Despawn();
		}
		return;
	}
	if (bHasSpawned && !Definition->bRespawn)
	{
		return;
	}
	if (Definition->SpawnWhen.Evaluate(Context))
	{
		Spawn();
	}
}

void AFVSpawner::Spawn()
{
	UClass* Class = Definition->ActorClass.LoadSynchronous();
	if (!Class)
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnedActor = GetWorld()->SpawnActor<AActor>(Class, GetActorTransform(), Params);
	if (!SpawnedActor)
	{
		return;
	}
	bHasSpawned = true;
	SpawnedActor->OnDestroyed.AddDynamic(this, &AFVSpawner::HandleSpawnedDestroyed);
	OnSpawned.Broadcast();
}

void AFVSpawner::Despawn()
{
	if (!SpawnedActor)
	{
		return;
	}
	AActor* Actor = SpawnedActor;
	Actor->OnDestroyed.RemoveDynamic(this, &AFVSpawner::HandleSpawnedDestroyed);
	SpawnedActor = nullptr;
	Actor->Destroy();
	OnDespawned.Broadcast();
}

void AFVSpawner::HandleSpawnedDestroyed(AActor* Actor)
{
	SpawnedActor = nullptr;
	OnDespawned.Broadcast();
}

#undef LOCTEXT_NAMESPACE