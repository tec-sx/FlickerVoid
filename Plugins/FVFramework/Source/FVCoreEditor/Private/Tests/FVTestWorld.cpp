#include "Tests/FVTestWorld.h"

#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

FFVTestWorld::FFVTestWorld()
{
	UGameInstance* Instance = NewObject<UGameInstance>(GEngine);
	GameInstance.Reset(Instance);

	// Creates a game world with its own world context and initializes the game instance subsystems.
	Instance->InitializeStandalone();

	UWorld* World = Instance->GetWorld();
	World->InitializeActorsForPlay(FURL());
	World->SetBegunPlay(true);
}

FFVTestWorld::~FFVTestWorld()
{
	UGameInstance* Instance = GameInstance.Get();
	UWorld* World = Instance->GetWorld();

	Instance->Shutdown();
	GameInstance.Reset();

	if (World)
	{
		World->BeginTearingDown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
}

UWorld* FFVTestWorld::GetWorld() const
{
	return GameInstance->GetWorld();
}

AActor* FFVTestWorld::SpawnActor() const
{
	UWorld* World = GetWorld();
	return World->SpawnActor<AActor>();
}

void FFVTestWorld::RegisterComponent(AActor* Actor, UActorComponent* Component)
{
	// The actor has begun play, so registering also initializes the component and begins its play.
	Actor->AddInstanceComponent(Component);
	Component->RegisterComponent();
}
