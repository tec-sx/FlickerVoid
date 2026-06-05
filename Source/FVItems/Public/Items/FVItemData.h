// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "FVItemData.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UTexture2D;
class AActor;

/**
 * BASE ITEM EFFECT SYSTEM
 * 
 * Item effects are modular, composable components that define what happens when an item is used.
 * Effects can be:
 * - Created in C++ for performance-critical or complex logic
 * - Created in Blueprint for designer-friendly custom effects
 * - Combined on a single item (e.g., alcoholic beverage that restores energy)
 * 
 * All effects are applied through the Gameplay Ability System via Gameplay Effects.
 */

/**
 * Base class for all item effects
 * Extend this in Blueprint to create custom item effects without C++ changes
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class UE_API UFVItemEffect : public UObject
{
	GENERATED_BODY()

public:
	/** Display name for this effect (used in UI) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FText EffectName;

	/** Description of what this effect does */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (MultiLine = true))
	FText EffectDescription;

	/** Tags that describe this effect (e.g., Effect.Consumable.Food, Effect.Status.Intoxication) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FGameplayTagContainer EffectTags;

	/**
	 * Called when the item is used by an actor
	 * Override in C++ or Blueprint to implement custom behavior
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Effect")
	void ApplyEffect(AActor* User, int32 ItemQuantity);

	/**
	 * Check if this effect can be applied to the given actor
	 * Override to add custom requirements (e.g., check for tags, stats, etc.)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Effect")
	bool CanApplyEffect(AActor* User) const;

	virtual void ApplyEffect_Implementation(AActor* User, int32 ItemQuantity) {}
	virtual bool CanApplyEffect_Implementation(AActor* User) const { return true; }
};

/**
 * CONSUMABLE EFFECTS - Food, Beverages, Drugs
 */

/**
 * Base consumable effect - provides energy/nutrition
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UE_API UFVConsumableEffect : public UFVItemEffect
{
	GENERATED_BODY()

public:
	/** How many charges/uses this item has (1 for single-use) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "1"))
	int32 Charges = 1;

	/** Base strength/potency of the consumable effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0"))
	float EffectStrength = 50.0f;

	UFVConsumableEffect()
	{
		EffectName = FText::FromString("Consumable");
		EffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Consumable")));
	}
};

/**
 * Food effect - affects player physique through nutrition
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UE_API UFVFoodEffect : public UFVConsumableEffect
{
	GENERATED_BODY()

public:
	/** Energy provided (calories, restores stamina) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Food|Nutrition", meta = (ClampMin = "0.0"))
	float Energy = 100.0f;

	/** Fat content (affects weight gain/loss over time) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Food|Nutrition", meta = (ClampMin = "0.0"))
	float Fat = 0.0f;

	/** Sugar content (quick energy, affects metabolism) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Food|Nutrition", meta = (ClampMin = "0.0"))
	float Sugar = 0.0f;

	/** Protein content (affects muscle mass, recovery) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Food|Nutrition", meta = (ClampMin = "0.0"))
	float Protein = 0.0f;

	UFVFoodEffect()
	{
		EffectName = FText::FromString("Food");
		EffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Consumable.Food")));
	}
};

/**
 * Beverage effect - can provide hydration and/or intoxication
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UE_API UFVBeverageEffect : public UFVConsumableEffect
{
	GENERATED_BODY()

public:
	/** Is this beverage alcoholic? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beverage")
	bool bIsAlcoholic = false;

	/** Energy provided (for energy drinks, sugary beverages) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beverage", meta = (ClampMin = "0.0"))
	float Energy = 0.0f;

	/** Hydration amount restored */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beverage", meta = (ClampMin = "0.0"))
	float Hydration = 50.0f;

	/** Intoxication strength (alcohol level, affects sanity and perception) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beverage|Intoxication", 
		meta = (ClampMin = "0.0", ClampMax = "100.0", EditCondition = "bIsAlcoholic"))
	float IntoxicationStrength = 0.0f;

	/** Risk of developing addiction (0-100, cumulative over time) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beverage|Addiction", 
		meta = (ClampMin = "0.0", ClampMax = "100.0", EditCondition = "bIsAlcoholic"))
	float AddictionRisk = 0.0f;

	UFVBeverageEffect()
	{
		EffectName = FText::FromString("Beverage");
		EffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Consumable.Beverage")));
	}
};

/**
 * Drug effect - affects sanity, perception, and mental state
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UE_API UFVDrugEffect : public UFVConsumableEffect
{
	GENERATED_BODY()

public:
	/** Intoxication/high strength (affects sanity, perception, and behavior) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drug|Effects", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float IntoxicationStrength = 50.0f;

	/** Risk of developing addiction (0-100, cumulative) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drug|Addiction", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float AddictionRisk = 30.0f;

	/** Duration of the drug effect in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drug|Effects", meta = (ClampMin = "0.0"))
	float EffectDuration = 300.0f;

	/** Does this drug provide temporary mental clarity or focus? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drug|Effects")
	bool bProvidesMentalClarity = false;

	/** Sanity modifier (positive or negative) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drug|Sanity", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
	float SanityModifier = -10.0f;

	UFVDrugEffect()
	{
		EffectName = FText::FromString("Drug");
		EffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Consumable.Drug")));
	}
};

/**
 * EQUIPMENT EFFECTS - Clothing, Armor, Accessories
 */

/**
 * Equipment effect - for wearable items with durability
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UE_API UFVEquipmentEffect : public UFVItemEffect
{
	GENERATED_BODY()

public:
	/** Socket to equip this item to (e.g., "Head", "Chest", "Hands") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FName EquipSocket = NAME_None;

	/** Maximum durability of this equipment */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability", meta = (ClampMin = "0.0"))
	float MaxDurability = 100.0f;

	/** How much durability is lost per use/time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability", meta = (ClampMin = "0.0"))
	float WearRate = 1.0f;

	/** Armor/protection value */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats", meta = (ClampMin = "0.0"))
	float ArmorValue = 0.0f;

	/** Tags granted to the wearer while equipped (e.g., Character.Outfit.Formal, Character.Outfit.Casual) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Tags")
	FGameplayTagContainer GrantedTags;

	UFVEquipmentEffect()
	{
		EffectName = FText::FromString("Equipment");
		EffectTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.Equipment")));
	}
};

/**
 * Modular action definition for items
 * Actions are driven by gameplay tags (e.g., Item.Action.Consume, Item.Action.Equip)
 */
USTRUCT(BlueprintType)
struct FFVItemAction
{
	GENERATED_BODY()

	/** Display name of the action (e.g., "Drink", "Equip", "Read") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	FText ActionName;

	/** Tag that identifies this action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	FGameplayTag ActionTag;

	/** Icon for this action in UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	TObjectPtr<UTexture2D> ActionIcon;

	/** Should this action consume the item? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
	bool bConsumesItem = false;

	FFVItemAction()
		: ActionName(FText::GetEmpty())
		, bConsumesItem(false)
	{}
};

/**
 * Core item data structure - This represents the data template for an item
 * All instances of an item type reference this structure
 * 
 * TAG-DRIVEN SYSTEM:
 * - Categories: Use tags like Item.Category.Weapon, Item.Category.Consumable
 * - Subcategories: Item.Category.Weapon.Melee.Sword, Item.Category.Consumable.Potion.Health
 * - Rarity: Item.Rarity.Common, Item.Rarity.Legendary
 * - Types: Item.Type.Alchemical, Item.Type.Questable
 * 
 * EFFECT SYSTEM:
 * - Items can have multiple effects (e.g., alcoholic beverage that also restores energy)
 * - Effects are Blueprint-extensible - designers can create new effect types
 * - Effects are applied when the item is used/consumed/equipped
 */
USTRUCT(BlueprintType)
struct FFVItemDataTemplate
{
	GENERATED_BODY()

	/** Unique identifier for this item type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	/** Display name of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	/** Item description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemDescription;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon;

	/** 3D mesh for world representation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMesh> WorldMesh;

	/** Maximum stack size (1 = not stackable) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 MaxStackSize;

	/** Weight of a single item unit */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float Weight;

	/** Tags required for the character to use this item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Requirements")
	FGameplayTagContainer RequiredTags;

	/** Tags that must NOT be present for the item to be usable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Requirements")
	FGameplayTagContainer BlockedTags;

	/** 
	 * Tags associated with this item - FULLY MODULAR SYSTEM
	 * 
	 * Categories/Subcategories (hierarchical):
	 * - Item.Category.Clothing.Shirt
	 * - Item.Category.Consumable.Beverage.Alcoholic
	 * - Item.Category.Consumable.Food.FastFood
	 * - Item.Category.Consumable.Drug.Prescription
	 * - Item.Category.Crafting.Metal.Iron
	 * - Item.Category.Quest.MainStory
	 * 
	 * Rarity:
	 * - Item.Rarity.Common
	 * - Item.Rarity.Uncommon
	 * - Item.Rarity.Rare
	 * - Item.Rarity.Epic
	 * - Item.Rarity.Legendary
	 * 
	 * Type/Properties:
	 * - Item.Type.Technological
	 * - Item.Property.Stackable
	 * - Item.Property.Tradeable
	 * - Item.Property.Questable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Tags")
	FGameplayTagContainer ItemTags;

	/**
	 * Effects applied when this item is used
	 * Can have multiple effects (e.g., food that provides energy and affects physique)
	 * Create new effect types in Blueprint for custom behavior
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Item|Effects")
	TArray<TObjectPtr<UFVItemEffect>> ItemEffects;

	/** Available actions for this item (modular system) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Actions")
	TArray<FFVItemAction> AvailableActions;

	/** Value in currency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Economy")
	int32 Value;

	/** Can this item be dropped? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Flags")
	bool bCanDrop;

	/** Can this item be sold? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Flags")
	bool bCanSell;

	/** Can this item be examined/inspected? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Flags")
	bool bCanInspect;

	/** Can this item be used from inventory? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Flags")
	bool bCanUse;

	/** Does this item have durability? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Flags")
	bool bHasDurability;

	FFVItemDataTemplate()
		: ItemID(NAME_None)
		, MaxStackSize(1)
		, Weight(0.0f)
		, Value(0)
		, bCanDrop(true)
		, bCanSell(true)
		, bCanInspect(true)
		, bCanUse(false)
		, bHasDurability(false)
	{}

	/**
	 * Helper function to get the primary category tag
	 * Returns the first tag matching "Item.Category.*"
	 */
	UE_API FGameplayTag GetPrimaryCategoryTag() const
	{
		FGameplayTag CategoryParent = FGameplayTag::RequestGameplayTag(FName("Item.Category"));
		
		for (const FGameplayTag& Tag : ItemTags)
		{
			if (Tag.MatchesTag(CategoryParent))
			{
				return Tag;
			}
		}
		
		return FGameplayTag();
	}

	/**
	 * Helper function to get the rarity tag
	 * Returns the first tag matching "Item.Rarity.*"
	 */
	UE_API FGameplayTag GetRarityTag() const
	{
		FGameplayTag RarityParent = FGameplayTag::RequestGameplayTag(FName("Item.Rarity"));
		
		for (const FGameplayTag& Tag : ItemTags)
		{
			if (Tag.MatchesTag(RarityParent))
			{
				return Tag;
			}
		}
		
		return FGameplayTag();
	}

	/**
	 * Check if item belongs to a specific category
	 */
	UE_API bool IsInCategory(const FGameplayTag& CategoryTag) const
	{
		return ItemTags.HasTag(CategoryTag);
	}

	/**
	 * Get all effects of a specific type
	 * Example: GetEffectsOfType<UFVFoodEffect>()
	 */
	template<typename TEffectType>
	TArray<TEffectType*> GetEffectsOfType() const
	{
		TArray<TEffectType*> Result;
		for (UFVItemEffect* Effect : ItemEffects)
		{
			if (TEffectType* TypedEffect = Cast<TEffectType>(Effect))
			{
				Result.Add(TypedEffect);
			}
		}
		return Result;
	}
};

/**
 * Instance of an item in inventory - stores runtime data about a specific item
 */
USTRUCT(BlueprintType)
struct FFVItemInstance
{
	GENERATED_BODY()

	/** Reference to the item template data */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<class UFVItemDataAsset> ItemTemplate = nullptr;

	/** Current quantity of this item */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 Quantity = 0;

	/** Unique instance ID for tracking individual items */
	UPROPERTY(BlueprintReadOnly, Category = "Item", meta = (IgnoreForMemberInitializationTest))
	FGuid InstanceID;

	/** Custom tags applied to this specific instance (e.g., "Instance.Enchanted", "Instance.Damaged") */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FGameplayTagContainer InstanceTags;

	/** Durability (0.0 - 1.0) for items that degrade */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	float Durability = 1.0f;

	FFVItemInstance()
		: InstanceID(FGuid::NewGuid())
	{
	}

	bool IsValid() const { return ItemTemplate != nullptr; }
};

#undef UE_API
