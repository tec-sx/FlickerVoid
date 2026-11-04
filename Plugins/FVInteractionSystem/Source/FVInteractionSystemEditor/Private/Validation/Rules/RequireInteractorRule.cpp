#include "Validation/Rules/RequireInteractorRule.h"

#include "Components/ActorComponent.h"
#include "Components/FVInteractorComponent.h"

static const FName RequiresInteractorMeta(TEXT("RequiresInteractor"));

void FRequireInteractorRule::Validate(const FInteractionCompileContext& Context) const
{
	const bool bHasInteractor = Context.Components.ContainsByPredicate([](const UActorComponent* Component)
	{
		return Component->IsA<UFVInteractorComponent>();
	});

	if (bHasInteractor)
	{
		return;
	}

	for (const UActorComponent* Component : Context.Components)
	{
		const UClass* Class = Component->GetClass();
		if (Class->HasMetaData(RequiresInteractorMeta))
		{
			Context.Error(FString::Printf(TEXT("Component of type '%s' requires a UFVInteractorComponent on the same actor."), *Class->GetName()));
		}
	}
}
