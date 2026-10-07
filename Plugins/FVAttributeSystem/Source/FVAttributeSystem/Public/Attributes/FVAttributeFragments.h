#pragma once

#include "CoreMinimal.h"
#include "Attributes/FVAttributeDefinition.h"
#include "Data/FVCharacterDefinition.h"
#include "FVAttributeFragments.generated.h"

/** Starting attributes for a character definition. */
USTRUCT(BlueprintType, meta = (DisplayName = "Attributes"))
struct FVATTRIBUTESYSTEM_API FFVCharacterFragment_Attributes : public FFVCharacterFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Attributes")
	TObjectPtr<const UFVAttributeSetDefinition> AttributeSet;

	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (TitleProperty = "Attribute"))
	TArray<FFVAttributeValue> Overrides;
};
