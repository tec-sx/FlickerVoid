#include "Validation/FVInteractionActorValidator.h"

#include "Engine/Blueprint.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/DataValidation.h"
#include "Validation/InteractionBlueprintComponentUtils.h"
#include "Validation/InteractionCompileRule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionActorValidator)

namespace
{
	const UClass* GetBlueprintActorClass(const UObject* Object)
	{
		const UBlueprint* Blueprint = Cast<UBlueprint>(Object);
		const UClass* GeneratedClass = Blueprint ? Blueprint->GeneratedClass.Get() : nullptr;
		if (GeneratedClass && GeneratedClass->IsChildOf<AActor>())
		{
			return GeneratedClass;
		}
		return nullptr;
	}

	void CollectActorComponents(const AActor& Actor, TArray<const UActorComponent*>& OutComponents)
	{
		for (const UActorComponent* Component : Actor.GetComponents())
		{
			if (Component)
			{
				OutComponents.Add(Component);
			}
		}
	}
}

bool UFVInteractionActorValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	if (GetBlueprintActorClass(InObject))
	{
		return true;
	}
	return InObject && (InObject->IsA<UWorld>() || InObject->IsA<AActor>());
}

EDataValidationResult UFVInteractionActorValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	TArray<const AActor*> Actors;
	TArray<const UActorComponent*> Templates;

	if (const UClass* ActorClass = GetBlueprintActorClass(InAsset))
	{
		InteractionBlueprintComponentUtils::CollectComponentTemplates(ActorClass, Templates);
	}
	else if (const UWorld* World = Cast<UWorld>(InAsset))
	{
		const ULevel* Level = World->PersistentLevel;
		if (Level)
		{
			for (const AActor* Actor : Level->Actors)
			{
				Actors.Add(Actor);
			}
		}
	}
	else if (const AActor* Actor = Cast<AActor>(InAsset))
	{
		Actors.Add(Actor);
	}

	int32 ErrorCount = 0;
	auto Run = [this, InAsset, &ErrorCount](const TArray<const UActorComponent*>& Components, const FString& Prefix)
	{
		const FInteractionCompileContext RuleContext(Components, [this, InAsset, &ErrorCount, &Prefix](const EMessageSeverity::Type Severity, const FString& Message)
		{
			const FText Text = FText::FromString(Prefix + Message);
			if (Severity == EMessageSeverity::Error)
			{
				++ErrorCount;
				AssetFails(InAsset, Text);
				return;
			}
			AssetWarning(InAsset, Text);
		});
		InteractionBlueprintComponentUtils::RunRules(RuleContext);
	};

	if (!Templates.IsEmpty())
	{
		Run(Templates, FString());
	}

	for (const AActor* Actor : Actors)
	{
		if (!Actor)
		{
			continue;
		}

		TArray<const UActorComponent*> Components;
		CollectActorComponents(*Actor, Components);
		const FString Prefix = FString::Printf(TEXT("%s: "), *Actor->GetActorNameOrLabel());
		Run(Components, Prefix);
	}

	if (ErrorCount > 0)
	{
		return EDataValidationResult::Invalid;
	}
	AssetPasses(InAsset);
	return EDataValidationResult::Valid;
}
