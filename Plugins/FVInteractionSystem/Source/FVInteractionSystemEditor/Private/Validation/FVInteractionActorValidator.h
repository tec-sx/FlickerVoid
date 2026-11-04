#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "FVInteractionActorValidator.generated.h"

/**
 * Runs the interaction rules through Data Validation: on actor Blueprints (inherited components included), on levels'
 * placed actors and on external actor packages, so actors built in AngelScript are checked where they are placed.
 */
UCLASS()
class UFVInteractionActorValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};
