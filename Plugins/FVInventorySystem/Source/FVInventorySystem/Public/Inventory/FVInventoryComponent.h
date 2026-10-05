#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FVInventoryComponent.generated.h"

class UFVItemDefinition;

USTRUCT(BlueprintType)
struct FVINVENTORYSYSTEM_API FFVItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (ClampMin = 1))
	int32 Quantity = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnInventoryItemChanged, UFVItemDefinition*, Item, int32, OldQuantity, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnInventoryItemUsed, UFVItemDefinition*, Item);

/** Holds items, one entry per item definition. Takes items from a UFVItemReceiverComponent on the same actor. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVINVENTORYSYSTEM_API UFVInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInventoryComponent();

	static UFVInventoryComponent* Find(const AActor* Actor);

	/** Returns the quantity actually added (limited by MaxQuantity). */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	int32 AddItem(UFVItemDefinition* Item, int32 Quantity = 1);

	/** Removes nothing and returns false if there isn't enough. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	bool RemoveItem(UFVItemDefinition* Item, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	int32 GetQuantity(const UFVItemDefinition* Item) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool HasItem(const UFVItemDefinition* Item, int32 Quantity = 1) const { return GetQuantity(Item) >= Quantity; }

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	const TArray<FFVItemStack>& GetItems() const { return Items; }

	/** Items whose category matches the tag (children included). */
	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	TArray<FFVItemStack> GetItemsInCategory(FGameplayTag Category) const;

	UFUNCTION(BlueprintPure, Category = "FV|Inventory")
	bool CanUseItem(const UFVItemDefinition* Item) const;

	/** Applies the item's Usable fragment and consumes one if configured. */
	UFUNCTION(BlueprintCallable, Category = "FV|Inventory")
	bool UseItem(UFVItemDefinition* Item);

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemChanged OnItemChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Inventory")
	FFVOnInventoryItemUsed OnItemUsed;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Starting contents; also the saved state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory", meta = (TitleProperty = "Item"))
	TArray<FFVItemStack> Items;

private:
	UFUNCTION()
	void HandleItemReceived(UFVItemDefinition* Item, int32 Quantity, AActor* Source);

	FFVItemStack* FindStack(const UFVItemDefinition* Item);
	void SetQuantity(UFVItemDefinition* Item, int32 NewQuantity);
};
