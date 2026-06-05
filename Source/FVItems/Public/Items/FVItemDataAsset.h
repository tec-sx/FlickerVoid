// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FVItemData.h"
#include "FVItemDataAsset.generated.h"

#define UE_API FLICKERVOIDITEMS_API

/**
 * Data asset for defining item templates
 * Create instances of this in content browser for each item type
 */
UCLASS(MinimalAPI, BlueprintType)
class UFVItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	
	/** Blueprint-defined behavior for using the item */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void OnItemUsed(AActor* User);

	/** Blueprint-defined behavior for inspecting the item */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void OnItemInspected(AActor* Inspector);

	/**
	 * Get the primary category tag (e.g., "Item.Category.Weapon")
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	UE_API FGameplayTag GetPrimaryCategoryTag() const;

	/**
	 * Get the rarity tag (e.g., "Item.Rarity.Legendary")
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	UE_API FGameplayTag GetRarityTag() const;

	/**
	 * Check if item belongs to a specific category
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	UE_API bool IsInCategory(const FGameplayTag& CategoryTag) const;

	/**
	 * Check if item has a specific tag
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	UE_API bool HasItemTag(const FGameplayTag& Tag) const;

	/**
	 * Get all actions available for this item
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	UE_API const TArray<FFVItemAction>& GetAvailableActions() const { return ItemData.AvailableActions; }

	/**
	 * Find a specific action by tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	UE_API bool FindActionByTag(FGameplayTag ActionTag, FFVItemAction& OutAction) const;

	/**
	 * Check if item has a specific action available
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	UE_API bool HasAction(FGameplayTag ActionTag) const;

#if WITH_EDITOR
	/**
	 * Validate item data in editor
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	/**
	 * Get asset description for editor
	 */
	virtual FString GetDesc() override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FFVItemDataTemplate ItemData;
};

#undef UE_API
