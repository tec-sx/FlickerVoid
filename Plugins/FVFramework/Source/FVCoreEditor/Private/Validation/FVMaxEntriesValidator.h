#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "FVMaxEntriesValidator.generated.h"

/**
 * Warns when an array property tagged meta = (MaxEntries = N) holds more than N entries. Checks data assets and the
 * class defaults of Blueprints, including those of script classes (the game's rule of three: at most 3 of anything).
 */
UCLASS()
class UFVMaxEntriesValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};
