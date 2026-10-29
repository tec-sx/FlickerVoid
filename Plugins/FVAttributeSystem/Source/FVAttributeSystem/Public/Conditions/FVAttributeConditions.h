#pragma once

#include "CoreMinimal.h"
#include "Attributes/FVAttributeDefinition.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "FVAttributeConditions.generated.h"

class UGameplayEffect;

UENUM(BlueprintType, meta = (ScriptName = "AttributeCompare"))
enum class EFVCompare : uint8
{
	AtLeast UMETA(ToolTip = "Value is greater than or equal to the threshold."),
	AtMost UMETA(ToolTip = "Value is less than or equal to the threshold."),
	Equal UMETA(ToolTip = "Value equals the threshold.")
};

USTRUCT(BlueprintType, meta = (DisplayName = "Attribute"))
struct FVATTRIBUTESYSTEM_API FFVCondition_Attribute : public FFVConditionBase
{
	GENERATED_BODY()

	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVCompare Compare = EFVCompare::AtLeast;

	UPROPERTY(EditAnywhere, Category = "Condition")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
	virtual bool GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Modify Attribute"))
struct FVATTRIBUTESYSTEM_API FFVEffect_ModifyAttribute : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ToolTip = "Added to the base value permanently."))
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, Category = "Effect")
	float Delta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bApplyToTarget = false;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Set Attribute"))
struct FVATTRIBUTESYSTEM_API FFVEffect_SetAttribute : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditAnywhere, Category = "Effect")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bApplyToTarget = false;
};

/** Applies a Gameplay Effect (perk, kink, status, outfit bonus) to the instigator or target's ability system. */
USTRUCT(BlueprintType, meta = (DisplayName = "Apply Gameplay Effect"))
struct FVATTRIBUTESYSTEM_API FFVEffect_ApplyGameplayEffect : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TSubclassOf<UGameplayEffect> Effect;

	UPROPERTY(EditAnywhere, Category = "Effect")
	float Level = 1.f;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bApplyToTarget = false;
};

/** Removes every active instance of a Gameplay Effect, e.g. a status that ends when a quest step completes. */
USTRUCT(BlueprintType, meta = (DisplayName = "Remove Gameplay Effect"))
struct FVATTRIBUTESYSTEM_API FFVEffect_RemoveGameplayEffect : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TSubclassOf<UGameplayEffect> Effect;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bApplyToTarget = false;
};
