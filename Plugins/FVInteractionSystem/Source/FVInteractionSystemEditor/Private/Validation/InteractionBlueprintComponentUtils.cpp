#include "Validation/InteractionBlueprintComponentUtils.h"

#include "Components/ActorComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/InheritableComponentHandler.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"
#include "Validation/InteractionCompileRule.h"
#include "Validation/InteractionCompileRuleRegistry.h"

namespace
{
	/** The template a node spawns in ActorClass: the most derived child Blueprint's override, or the node's own template. */
	const UActorComponent* GetEffectiveTemplate(const UClass* ActorClass, const UBlueprintGeneratedClass* Owner, const USCS_Node* Node)
	{
		const FComponentKey Key(Node);
		const UBlueprintGeneratedClass* Current = Cast<UBlueprintGeneratedClass>(ActorClass);
		while (Current && Current != Owner)
		{
			UBlueprintGeneratedClass* Mutable = const_cast<UBlueprintGeneratedClass*>(Current);
			const UInheritableComponentHandler* Handler = Mutable->GetInheritableComponentHandler(false);
			const UActorComponent* Override = Handler ? Handler->GetOverridenComponentTemplate(Key) : nullptr;
			if (Override)
			{
				return Override;
			}
			Current = Cast<UBlueprintGeneratedClass>(Current->GetSuperClass());
		}
		return Node->ComponentTemplate;
	}
}

void InteractionBlueprintComponentUtils::CollectComponentTemplates(const UClass* ActorClass, bool bWithClassDefaults, TArray<const UActorComponent*>& OutTemplates)
{
	OutTemplates.Reset();
	if (!ActorClass)
	{
		return;
	}

	const UObject* Defaults = bWithClassDefaults ? ActorClass->GetDefaultObject() : nullptr;
	if (const AActor* DefaultActor = Cast<AActor>(Defaults))
	{
		for (const UActorComponent* Component : DefaultActor->GetComponents())
		{
			if (Component)
			{
				OutTemplates.Add(Component);
			}
		}
	}

	const UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(ActorClass);
	while (BlueprintClass)
	{
		const USimpleConstructionScript* ConstructionScript = BlueprintClass->SimpleConstructionScript;
		if (ConstructionScript)
		{
			for (const USCS_Node* Node : ConstructionScript->GetAllNodes())
			{
				if (!Node)
				{
					continue;
				}

				const UActorComponent* Template = GetEffectiveTemplate(ActorClass, BlueprintClass, Node);
				if (Template)
				{
					OutTemplates.Add(Template);
				}
			}
		}
		BlueprintClass = Cast<UBlueprintGeneratedClass>(BlueprintClass->GetSuperClass());
	}
}

void InteractionBlueprintComponentUtils::RunRules(const FInteractionCompileContext& Context)
{
	for (const TSharedRef<IInteractionCompileRule>& Rule : FInteractionCompileRuleRegistry::Get().GetRules())
	{
		if (Rule->ShouldRun(Context))
		{
			Rule->Validate(Context);
		}
	}
}
