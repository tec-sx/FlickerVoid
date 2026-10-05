#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "FVAttributeDefinition.generated.h"

/**
 * One attribute (Perception, Stealth, Health, ...). New attributes are new data assets, so game
 * modules, AngelScript and Blueprints can add them without changing C++.
 */
UCLASS(BlueprintType)
class FVATTRIBUTESYSTEM_API UFVAttributeDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** Groups attributes in the UI and in debug output, e.g. Attribute.Skill or Attribute.Vital. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute", meta = (Categories = "Attribute"))
	FGameplayTag Category;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	float DefaultValue = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	float Min = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	float Max = 100.f;

	/** Rounds the final value, for attributes used as whole numbers. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	bool bInteger = false;

	float Clamp(float Value) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};

/** Initial value for one attribute, used by attribute sets and character fragments. */
USTRUCT(BlueprintType)
struct FVATTRIBUTESYSTEM_API FFVAttributeValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	float Value = 0.f;
};

/** A reusable block of starting attributes, e.g. "Civilian" or "Guard". */
UCLASS(BlueprintType)
class FVATTRIBUTESYSTEM_API UFVAttributeSetDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** Applied in order, so a later set overrides an earlier one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Set")
	TArray<TObjectPtr<const UFVAttributeSetDefinition>> Parents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Set", meta = (TitleProperty = "Attribute"))
	TArray<FFVAttributeValue> Attributes;

	/** Flattens Parents and this set into one list. */
	void Collect(TArray<FFVAttributeValue>& OutValues) const;
};
