#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "GameplayTagContainer.h"
#include "FVInventoryConditions.generated.h"

class UFVItemDefinition;

/** Instigator (or Target) holds at least Quantity of the item. */
USTRUCT(BlueprintType, meta = (DisplayName = "Has Item"))
struct FVINVENTORYSYSTEM_API FFVCondition_HasItem : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** Instigator (or Target) wears the item, or anything granting the tag when Item is empty. */
USTRUCT(BlueprintType, meta = (DisplayName = "Is Wearing"))
struct FVINVENTORYSYSTEM_API FFVCondition_IsWearing : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (EditCondition = "Item == nullptr"))
	FGameplayTag GrantedTag;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** Gives items to the Instigator (or Target), optionally equipping them. */
USTRUCT(BlueprintType, meta = (DisplayName = "Give Item"))
struct FVINVENTORYSYSTEM_API FFVEffect_GiveItem : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bEquip = false;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bGiveToTarget = false;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};

/** Removes items from the Instigator (or Target). */
USTRUCT(BlueprintType, meta = (DisplayName = "Take Item"))
struct FVINVENTORYSYSTEM_API FFVEffect_TakeItem : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bTakeFromTarget = false;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};
