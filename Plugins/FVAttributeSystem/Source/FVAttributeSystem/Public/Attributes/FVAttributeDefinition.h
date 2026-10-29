#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Data/FVDefinition.h"
#include "FVAttributeDefinition.generated.h"

/**
 * Display and range data for one Gameplay Ability System attribute (Perception, Stealth, Sanity...).
 * The value lives in an attribute set on the actor's ability system; Gameplay Effects change it.
 */
UCLASS(BlueprintType)
class FVATTRIBUTESYSTEM_API UFVAttributeDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (ToolTip = "Clamps to [Min, Max] and rounds when Integer is set."))
	float Clamp(float Value) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute", meta = (ToolTip = "The attribute set property this definition describes."))
	FGameplayAttribute Attribute;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute", meta = (Categories = "Attribute", ToolTip = "Groups attributes in the UI and debug output, e.g. Attribute.Skill or Attribute.Mental."))
	FGameplayTag Category;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute", meta = (ToolTip = "Lowest value; attribute sets deriving from FVAttributeSet clamp to it."))
	float Min = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute", meta = (ToolTip = "Highest value; attribute sets deriving from FVAttributeSet clamp to it."))
	float Max = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute", meta = (ToolTip = "Rounds the value, for attributes used as whole numbers."))
	bool bInteger = false;
};
