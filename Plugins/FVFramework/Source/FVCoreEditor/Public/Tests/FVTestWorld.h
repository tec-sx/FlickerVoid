#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/StrongObjectPtr.h"

class AActor;
class UGameInstance;
class UWorld;

/**
 * A standalone game instance and world for automation tests, so game instance subsystems (facts, quests) and actor
 * components run without Content or a map. Destroyed with the object.
 */
class FVCOREEDITOR_API FFVTestWorld
{
public:
	FFVTestWorld();
	~FFVTestWorld();

	FFVTestWorld(const FFVTestWorld&) = delete;
	FFVTestWorld& operator=(const FFVTestWorld&) = delete;

	UWorld* GetWorld() const;
	UGameInstance* GetGameInstance() const { return GameInstance.Get(); }

	/** An empty actor that has begun play, for tests that add components to it. */
	AActor* SpawnActor() const;

	/** Creates, registers and begins play on a component of the given class. */
	template <typename T>
	T* AddComponent(AActor* Actor) const
	{
		T* Component = NewObject<T>(Actor);
		RegisterComponent(Actor, Component);
		return Component;
	}

private:
	static void RegisterComponent(AActor* Actor, UActorComponent* Component);

	TStrongObjectPtr<UGameInstance> GameInstance;
};
