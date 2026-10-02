#include "Validation/Rules/RequireInteractorRule.h"

#include "Components/FVInteractorComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "Kismet2/CompilerResultsLog.h"

static const FName RequiresInteractorMeta(TEXT("RequiresInteractor"));

static void CollectComponentClasses(const UBlueprint* Blueprint, TArray<const UClass*>& OutClasses)
{
	for (; Blueprint; Blueprint = UBlueprint::GetBlueprintFromClass(Blueprint->ParentClass))
	{
		if (Blueprint->SimpleConstructionScript)
		{
			for (const USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
			{
				if (Node && Node->ComponentClass)
				{
					OutClasses.Add(Node->ComponentClass);
				}
			}
		}

		if (!UBlueprint::GetBlueprintFromClass(Blueprint->ParentClass))
		{
			if (const AActor* NativeDefault = Blueprint->ParentClass ? Cast<AActor>(Blueprint->ParentClass->GetDefaultObject()) : nullptr)
			{
				for (const UActorComponent* Component : NativeDefault->GetComponents())
				{
					if (Component)
					{
						OutClasses.Add(Component->GetClass());
					}
				}
			}
		}
	}
}

void FRequireInteractorRule::Validate(const FInteractionCompileContext& Context) const
{
	TArray<const UClass*> ComponentClasses;
	CollectComponentClasses(&Context.Blueprint, ComponentClasses);

	const bool bHasInteractor = ComponentClasses.ContainsByPredicate([](const UClass* Class)
	{
		return Class->IsChildOf(UFVInteractorComponent::StaticClass());
	});

	if (bHasInteractor)
	{
		return;
	}

	for (const UClass* Class : ComponentClasses)
	{
		if (Class->HasMetaData(RequiresInteractorMeta))
		{
			Context.MessageLog.Error(*FString::Printf(
				TEXT("Component of type '%s' requires a UFVInteractorComponent on the same actor."),
				*Class->GetName()));
		}
	}
}
