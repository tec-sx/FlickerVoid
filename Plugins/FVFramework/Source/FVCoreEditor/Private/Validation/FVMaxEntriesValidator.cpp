#include "Validation/FVMaxEntriesValidator.h"

#include "Engine/Blueprint.h"
#include "Misc/DataValidation.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMaxEntriesValidator)

#define LOCTEXT_NAMESPACE "FVMaxEntriesValidator"

namespace
{
	const FName MaxEntriesKey(TEXT("MaxEntries"));

	const UObject* GetValidatedObject(const UObject* Asset)
	{
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
		{
			return Blueprint->GeneratedClass ? Blueprint->GeneratedClass->GetDefaultObject() : nullptr;
		}
		return Asset;
	}

	bool HasLimitedArray(const UClass* Class)
	{
		for (TFieldIterator<FArrayProperty> It(Class); It; ++It)
		{
			if (It->HasMetaData(MaxEntriesKey))
			{
				return true;
			}
		}
		return false;
	}
}

bool UFVMaxEntriesValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	const UObject* Object = GetValidatedObject(InObject);
	return Object && HasLimitedArray(Object->GetClass());
}

EDataValidationResult UFVMaxEntriesValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UObject* Object = GetValidatedObject(InAsset);
	if (!Object)
	{
		return EDataValidationResult::NotValidated;
	}

	bool bWarned = false;
	for (TFieldIterator<FArrayProperty> It(Object->GetClass()); It; ++It)
	{
		const FArrayProperty* Property = *It;
		if (!Property->HasMetaData(MaxEntriesKey))
		{
			continue;
		}

		const int32 Limit = FCString::Atoi(*Property->GetMetaData(MaxEntriesKey));
		const int32 Count = FScriptArrayHelper(Property, Property->ContainerPtrToValuePtr<void>(Object)).Num();
		if (Count > Limit)
		{
			AssetWarning(InAsset, FText::Format(LOCTEXT("TooMany", "{0} has {1} entries; keep it to {2} or fewer."),
				Property->GetDisplayNameText(), Count, Limit));
			bWarned = true;
		}
	}

	if (!bWarned)
	{
		AssetPasses(InAsset);
	}
	return EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE
