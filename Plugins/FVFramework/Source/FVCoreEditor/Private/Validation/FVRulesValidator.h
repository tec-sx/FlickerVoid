#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "FVRulesValidator.generated.h"

/**
 * Reports condition sets and effect lists that can't work as set up: entries with no type, Object entries with nothing
 * picked, and each entry's setup error (a missing reference or tag). Checks data assets, Flow graphs and their nodes,
 * and the class defaults and component templates of Blueprints, including fragments held in instanced structs.
 */
UCLASS()
class UFVRulesValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};
