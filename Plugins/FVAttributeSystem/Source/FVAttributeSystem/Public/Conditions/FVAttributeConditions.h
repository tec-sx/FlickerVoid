#pragma once

#include "CoreMinimal.h"
#include "Attributes/FVAttributeComponent.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "FVAttributeConditions.generated.h"

UENUM(BlueprintType)
enum class EFVCompare : uint8
{
	AtLeast,
	AtMost,
	Equal
};

USTRUCT(BlueprintType, meta = (DisplayName = "Attribute"))
struct FVATTRIBUTESYSTEM_API FFVCondition_Attribute : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVCompare Compare = EFVCompare::AtLeast;

	UPROPERTY(EditAnywhere, Category = "Condition")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Modify Attribute"))
struct FVATTRIBUTESYSTEM_API FFVEffect_ModifyAttribute : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, Category = "Effect")
	float Delta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bApplyToTarget = false;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Set Attribute"))
struct FVATTRIBUTESYSTEM_API FFVEffect_SetAttribute : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, Category = "Effect")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bApplyToTarget = false;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};
