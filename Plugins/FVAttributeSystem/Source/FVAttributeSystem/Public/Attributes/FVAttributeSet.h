#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "FVAttributeSet.generated.h"

/**
 * Base for attribute sets whose attributes clamp to their FVAttributeDefinition (Min, Max, Integer).
 * Subclasses, in C++ or AngelScript, only declare FGameplayAttributeData properties.
 */
UCLASS(Abstract)
class FVATTRIBUTESYSTEM_API UFVAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

private:
	void ClampToDefinition(const FGameplayAttribute& Attribute, float& Value) const;
};
