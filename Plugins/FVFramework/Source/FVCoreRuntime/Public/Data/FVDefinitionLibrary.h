#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/FVDefinition.h"
#include "UObject/PrimaryAssetId.h"
#include "FVDefinitionLibrary.generated.h"

/** Finds and loads definitions through the Asset Manager. Each native definition class is a primary asset type, scanned through PrimaryAssetTypesToScan in DefaultGame.ini. */
UCLASS()
class FVCORERUNTIME_API UFVDefinitionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Definition", meta = (DeterminesOutputType = "DefinitionClass", ToolTip = "Every loaded definition of the class (and its subclasses) registered with the Asset Manager, sorted by asset name."))
	static TArray<UFVDefinition*> GetLoadedDefinitions(TSubclassOf<UFVDefinition> DefinitionClass);

	/** Loads every registered definition of the class in the background and keeps it loaded. OnLoaded may run before this returns. */
	static void LoadDefinitions(const UClass* DefinitionClass, const FSimpleDelegate& OnLoaded);

	/** The primary asset type of a definition class: the name of its closest native class. */
	static FPrimaryAssetType GetPrimaryAssetType(const UClass* DefinitionClass);

	template <typename T>
	static TArray<T*> GetLoadedDefinitions()
	{
		const TArray<UFVDefinition*> Definitions = GetLoadedDefinitions(T::StaticClass());
		TArray<T*> Result;
		Result.Reserve(Definitions.Num());
		for (UFVDefinition* Definition : Definitions)
		{
			Result.Add(CastChecked<T>(Definition));
		}
		return Result;
	}
};
