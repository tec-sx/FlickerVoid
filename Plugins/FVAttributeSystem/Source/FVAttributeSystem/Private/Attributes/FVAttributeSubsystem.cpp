#include "Attributes/FVAttributeSubsystem.h"

#include "FVAttributeSystem.h"
#include "Attributes/FVAttributeDefinition.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeSubsystem)

UFVAttributeSubsystem* UFVAttributeSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext != nullptr ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
	return GameInstance != nullptr ? GameInstance->GetSubsystem<UFVAttributeSubsystem>() : nullptr;
}

void UFVAttributeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!UAssetManager::IsInitialized())
	{
		return;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType(UFVAttributeDefinition::StaticClass()->GetFName()), AssetIds);

	if (AssetIds.IsEmpty())
	{
		UE_LOG(LogFVAttributeSystem, Warning, TEXT("No attribute definitions found. Add the FVAttributeDefinition primary asset type to the asset manager (DefaultGame.ini)."));
	}

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		const UFVAttributeDefinition* Definition = Cast<UFVAttributeDefinition>(AssetManager.GetPrimaryAssetPath(AssetId).TryLoad());
		if (Definition == nullptr || !Definition->Attribute.IsValid())
		{
			continue;
		}

		if (const UFVAttributeDefinition* const* Existing = DefinitionsByAttribute.Find(Definition->Attribute))
		{
			UE_LOG(LogFVAttributeSystem, Warning, TEXT("%s and %s both describe %s; using %s."),
				*GetNameSafe(*Existing), *GetNameSafe(Definition), *Definition->Attribute.GetName(), *GetNameSafe(*Existing));
			continue;
		}

		Definitions.Add(Definition);
		DefinitionsByAttribute.Add(Definition->Attribute, Definition);
	}
}

void UFVAttributeSubsystem::Deinitialize()
{
	Definitions.Reset();
	DefinitionsByAttribute.Reset();

	Super::Deinitialize();
}

const UFVAttributeDefinition* UFVAttributeSubsystem::FindDefinition(FGameplayAttribute Attribute) const
{
	const UFVAttributeDefinition* const* Found = DefinitionsByAttribute.Find(Attribute);
	return Found != nullptr ? *Found : nullptr;
}

TArray<UFVAttributeDefinition*> UFVAttributeSubsystem::GetDefinitions() const
{
	TArray<UFVAttributeDefinition*> Out;
	for (const TObjectPtr<const UFVAttributeDefinition>& Definition : Definitions)
	{
		Out.Add(const_cast<UFVAttributeDefinition*>(Definition.Get()));
	}
	return Out;
}
