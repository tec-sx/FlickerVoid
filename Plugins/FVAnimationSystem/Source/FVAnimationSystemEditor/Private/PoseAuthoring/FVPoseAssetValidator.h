#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "FVPoseAssetValidator.generated.h"

/**
 * Warns when a sided pose's counterpart no longer mirrors it, and when a pose's name and its changed bones
 * disagree on the side. A pose without a counterpart is fine: mirroring is opt-in.
 */
UCLASS()
class UFVPoseAssetValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};
