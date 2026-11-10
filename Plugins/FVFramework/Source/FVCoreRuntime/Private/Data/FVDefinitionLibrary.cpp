#include "Data/FVDefinitionLibrary.h"
#include "Engine/AssetManager.h"
#include "FVCoreRuntime.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDefinitionLibrary)

TArray<UFVDefinition*> UFVDefinitionLibrary::GetLoadedDefinitions(const TSubclassOf<UFVDefinition> DefinitionClass)
{
	TArray<UFVDefinition*> Result;
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager || !DefinitionClass)
	{
		return Result;
	}

	const FPrimaryAssetType Type = GetPrimaryAssetType(DefinitionClass);
	TArray<UObject*> Objects;
	AssetManager->GetPrimaryAssetObjectList(Type, Objects);
	for (UObject* Object : Objects)
	{
		if (Object && Object->IsA(DefinitionClass))
		{
			Result.Add(CastChecked<UFVDefinition>(Object));
		}
	}

	// The registry has no order; sorting by asset name keeps lists and tie breaks stable.
	Result.Sort([](const UFVDefinition& A, const UFVDefinition& B) { return A.GetFName().LexicalLess(B.GetFName()); });
	return Result;
}

void UFVDefinitionLibrary::LoadDefinitions(const UClass* DefinitionClass, const FSimpleDelegate& OnLoaded)
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager || !DefinitionClass)
	{
		OnLoaded.ExecuteIfBound();
		return;
	}

	const FPrimaryAssetType Type = GetPrimaryAssetType(DefinitionClass);
	FPrimaryAssetTypeInfo TypeInfo;
	if (!AssetManager->GetPrimaryAssetTypeInfo(Type, TypeInfo))
	{
		UE_LOG(LogFVCore, Warning, TEXT("%s isn't a primary asset type; add a PrimaryAssetTypesToScan entry for it in DefaultGame.ini."), *Type.ToString());
		OnLoaded.ExecuteIfBound();
		return;
	}

	TArray<FPrimaryAssetId> Ids;
	AssetManager->GetPrimaryAssetIdList(Type, Ids);
	if (Ids.IsEmpty())
	{
		OnLoaded.ExecuteIfBound();
		return;
	}

	const TArray<FName> NoBundles;
	AssetManager->LoadPrimaryAssets(Ids, NoBundles, OnLoaded);
}

FPrimaryAssetType UFVDefinitionLibrary::GetPrimaryAssetType(const UClass* DefinitionClass)
{
	const UClass* NativeClass = DefinitionClass;
	while (NativeClass && !NativeClass->HasAnyClassFlags(CLASS_Native))
	{
		NativeClass = NativeClass->GetSuperClass();
	}
	return NativeClass ? NativeClass->GetFName() : NAME_None;
}
