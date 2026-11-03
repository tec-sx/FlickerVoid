#include "Identity/FVIdentitySubsystem.h"

#include "Data/FVCharacterDefinition.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Identity/FVIdentityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVIdentitySubsystem)

UFVIdentitySubsystem* UFVIdentitySubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFVIdentitySubsystem>() : nullptr;
}

AActor* UFVIdentitySubsystem::FindActor(const UFVCharacterDefinition* Definition) const
{
	const TArray<AActor*> Actors = FindActors(Definition);
	return Actors.IsEmpty() ? nullptr : Actors[0];
}

TArray<AActor*> UFVIdentitySubsystem::FindActors(const UFVCharacterDefinition* Definition) const
{
	TArray<AActor*> Result;
	if (!Definition)
	{
		return Result;
	}

	const TObjectKey<UFVCharacterDefinition> Key(Definition);
	TArray<TWeakObjectPtr<UFVIdentityComponent>> Components;
	ComponentsByDefinition.MultiFind(Key, Components, true);
	for (const TWeakObjectPtr<UFVIdentityComponent>& Component : Components)
	{
		if (Component.IsValid())
		{
			Result.Add(Component->GetOwner());
		}
	}
	return Result;
}

void UFVIdentitySubsystem::Register(UFVIdentityComponent* Component)
{
	const UFVCharacterDefinition* Definition = Component ? Component->GetDefinition() : nullptr;
	if (Definition)
	{
		const TObjectKey<UFVCharacterDefinition> Key(Definition);
		const TWeakObjectPtr<UFVIdentityComponent> Value(Component);
		ComponentsByDefinition.AddUnique(Key, Value);
	}
}

void UFVIdentitySubsystem::Unregister(UFVIdentityComponent* Component)
{
	const UFVCharacterDefinition* Definition = Component ? Component->GetDefinition() : nullptr;
	if (Definition)
	{
		const TObjectKey<UFVCharacterDefinition> Key(Definition);
		const TWeakObjectPtr<UFVIdentityComponent> Value(Component);
		ComponentsByDefinition.Remove(Key, Value);
	}
}
