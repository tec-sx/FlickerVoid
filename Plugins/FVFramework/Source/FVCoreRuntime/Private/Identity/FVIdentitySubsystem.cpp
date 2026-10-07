#include "Identity/FVIdentitySubsystem.h"

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
	for (const TWeakObjectPtr<UFVIdentityComponent>& Component : Components)
	{
		if (Component.IsValid() && Definition && Component->GetDefinition() == Definition)
		{
			return Component->GetOwner();
		}
	}
	return nullptr;
}

TArray<AActor*> UFVIdentitySubsystem::FindActors(const UFVCharacterDefinition* Definition) const
{
	TArray<AActor*> Result;
	for (const TWeakObjectPtr<UFVIdentityComponent>& Component : Components)
	{
		if (Component.IsValid() && Definition && Component->GetDefinition() == Definition)
		{
			Result.Add(Component->GetOwner());
		}
	}
	return Result;
}

void UFVIdentitySubsystem::Register(UFVIdentityComponent* Component)
{
	Components.AddUnique(Component);
}

void UFVIdentitySubsystem::Unregister(UFVIdentityComponent* Component)
{
	Components.Remove(Component);
}
