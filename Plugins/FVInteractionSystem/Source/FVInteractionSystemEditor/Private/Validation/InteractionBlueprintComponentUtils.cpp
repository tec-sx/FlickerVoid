#include "Validation/InteractionBlueprintComponentUtils.h"

#include "Components/ActorComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"

namespace InteractionBlueprintComponentUtils
{
	bool HasComponentOfClass(const UBlueprint& Blueprint, const TSubclassOf<UActorComponent>& ComponentClass)
	{
		if (ComponentClass)
		{
			if (const USimpleConstructionScript* ConstructionScript = Blueprint.SimpleConstructionScript)
			{
				for (const USCS_Node* Node : ConstructionScript->GetAllNodes())
				{
					if (Node && Node->ComponentClass && Node->ComponentClass->IsChildOf(ComponentClass))
					{
						return true;
					}
				}
			}

			if (const UClass* ParentClass = Blueprint.ParentClass)
			{
				if (const AActor* ParentDefaultActor = Cast<AActor>(ParentClass->GetDefaultObject()))
				{
					return ParentDefaultActor->FindComponentByClass(ComponentClass) != nullptr;
				}
			}
		}

		return false;
	}

	void GetComponentTemplatesOfClass(const UBlueprint& Blueprint, const TSubclassOf<UActorComponent>& ComponentClass, TArray<const UActorComponent*>& OutTemplates)
	{
		OutTemplates.Reset();

		if (!ComponentClass)
		{
			return;
		}

		if (const USimpleConstructionScript* ConstructionScript = Blueprint.SimpleConstructionScript)
		{
			for (const USCS_Node* Node : ConstructionScript->GetAllNodes())
			{
				if (Node && Node->ComponentClass && Node->ComponentClass->IsChildOf(ComponentClass) && Node->ComponentTemplate)
				{
					OutTemplates.Add(Node->ComponentTemplate);
				}
			}
		}
	}
}
