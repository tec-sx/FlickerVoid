#pragma once

#include "CoreMinimal.h"
#include "FVInventoryItemBase.h"
#include "FVInventoryItemConsumable.generated.h"

/**
 * Inventory item class for consumable items (e.g., drinks, food, drugs)
 */

UCLASS()
class FLICKERVOIDITEMS_API UFVInventoryItemConsumable : public UFVInventoryItemBase
{
	GENERATED_BODY()

public:
	UPROPERTY(
		EditAnywhere, 
		BlueprintReadOnly, 
		Category = "Identification", 
		meta = (Categories = "Item.Consumable"))
	FGameplayTag ItemTag;

	UPROPERTY(
		EditDefaultsOnly, 
		BlueprintReadOnly, 
		Category = "ItemData",
		DisplayName = "Item Effect - Strength",
		meta = (Categories = "ItemEffect"))
	TMap<FGameplayTag, float> ItemEffects;

	// Functions
	// ----------------------------------------------------------------------------------
	virtual FGameplayTag GetTag() const override { return ItemTag; }
};