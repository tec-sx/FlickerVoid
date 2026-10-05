#include "FVDebugUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagsManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogFVDebug);

APawn* FVDebug::GetPlayerPawn(const UWorld* World)
{
	return World != nullptr ? UGameplayStatics::GetPlayerPawn(World, 0) : nullptr;
}

FGameplayTag FVDebug::FindTag(const FString& Name)
{
	const FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName(*Name), false);
	if (!Tag.IsValid())
	{
		UE_LOG(LogFVDebug, Warning, TEXT("Unknown gameplay tag '%s'."), *Name);
	}
	return Tag;
}

UFVDefinition* FVDebug::FindDefinition(const UClass* Class, const FString& IdOrName)
{
	if (Class == nullptr || IdOrName.IsEmpty())
	{
		return nullptr;
	}

	const IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetData> Assets;
	Registry.GetAssetsByClass(Class->GetClassPathName(), Assets, true);

	for (const FAssetData& Asset : Assets)
	{
		if (Asset.AssetName.ToString().Equals(IdOrName, ESearchCase::IgnoreCase))
		{
			return Cast<UFVDefinition>(Asset.GetAsset());
		}
	}

	for (const FAssetData& Asset : Assets)
	{
		const UFVDefinition* Definition = Cast<UFVDefinition>(Asset.GetAsset());
		if (Definition != nullptr && Definition->Id.ToString().Equals(IdOrName, ESearchCase::IgnoreCase))
		{
			return const_cast<UFVDefinition*>(Definition);
		}
	}

	UE_LOG(LogFVDebug, Warning, TEXT("No %s matches '%s'."), *Class->GetName(), *IdOrName);
	return nullptr;
}
