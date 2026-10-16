#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "GameplayTagContainer.h"
#include "FVInventoryConditions.generated.h"

class UFVItemDefinition;
class UFVLootTableDefinition;

/** Instigator (or Target) holds at least Quantity of the item. */
USTRUCT(BlueprintType, meta = (DisplayName = "Has Item"))
struct FVINVENTORYSYSTEM_API FFVCondition_HasItem : public FFVConditionBase
{
	GENERATED_BODY()

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;

public:
	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;
};

/** Instigator (or Target) holds at least Quantity units of items whose definition Tags contain the tag, e.g. a key or a tool. */
USTRUCT(BlueprintType, meta = (DisplayName = "Has Item With Tag"))
struct FVINVENTORYSYSTEM_API FFVCondition_HasItemWithTag : public FFVConditionBase
{
	GENERATED_BODY()

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;

public:
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ToolTip = "Matched against the definition Tags; children count, e.g. Key.Door matches Key.Door.Mansion."))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;
};

/** Instigator (or Target) has at least Amount money. */
USTRUCT(BlueprintType, meta = (DisplayName = "Has Money"))
struct FVINVENTORYSYSTEM_API FFVCondition_HasMoney : public FFVConditionBase
{
	GENERATED_BODY()

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;

public:
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = 1))
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;
};

/** Instigator (or Target) wears the item, or anything granting the tag when Item is empty. */
USTRUCT(BlueprintType, meta = (DisplayName = "Is Wearing"))
struct FVINVENTORYSYSTEM_API FFVCondition_IsWearing : public FFVConditionBase
{
	GENERATED_BODY()

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;

public:
	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (EditCondition = "Item == nullptr"))
	FGameplayTag GrantedTag;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckTarget = false;
};

/** Gives items to the Instigator (or Target), optionally equipping them. */
USTRUCT(BlueprintType, meta = (DisplayName = "Give Item"))
struct FVINVENTORYSYSTEM_API FFVEffect_GiveItem : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bEquip = false;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bGiveToTarget = false;
};

/** Removes items from the Instigator (or Target). */
USTRUCT(BlueprintType, meta = (DisplayName = "Take Item"))
struct FVINVENTORYSYSTEM_API FFVEffect_TakeItem : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 1))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bTakeFromTarget = false;
};

/** Gives money to the Instigator (or Target). */
USTRUCT(BlueprintType, meta = (DisplayName = "Give Money"))
struct FVINVENTORYSYSTEM_API FFVEffect_GiveMoney : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 1))
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bGiveToTarget = false;
};

/** Takes money from the Instigator (or Target); takes nothing if there isn't enough. */
USTRUCT(BlueprintType, meta = (DisplayName = "Take Money"))
struct FVINVENTORYSYSTEM_API FFVEffect_TakeMoney : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 1))
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bTakeFromTarget = false;
};

/** Rolls a loot table into the Instigator's (or Target's) inventory. Entry conditions see the effect's Instigator and the receiver as Target. */
USTRUCT(BlueprintType, meta = (DisplayName = "Give Loot"))
struct FVINVENTORYSYSTEM_API FFVEffect_GiveLoot : public FFVEffectBase
{
	GENERATED_BODY()

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVLootTableDefinition> Table;

	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bGiveToTarget = false;
};
