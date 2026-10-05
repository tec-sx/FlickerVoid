#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "Data/FVFragment.h"
#include "FVItemDefinition.generated.h"

/** Base for item fragments (equippable, usable, ...). */
USTRUCT(BlueprintType, meta = (Hidden))
struct FVINVENTORYSYSTEM_API FFVItemFragment : public FFVFragment
{
	GENERATED_BODY()
};

/** An item that can exist in the world and be carried. Behaviour is composed through fragments. */
UCLASS(BlueprintType)
class FVINVENTORYSYSTEM_API UFVItemDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (Categories = "Item"))
	FGameplayTag Category;

	/** Maximum quantity one inventory can hold. 0 = unlimited. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 0))
	int32 MaxQuantity = 0;

	/** Quest items can't be dropped or given away by the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bQuestItem = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVItemFragment>> Fragments;

	template <typename T>
	const T* FindFragment() const { return FVFragments::Find<T>(Fragments); }

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsEquippable() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsUsable() const;

	/** Equipment slot, or None if not equippable. */
	UFUNCTION(BlueprintPure, Category = "Item")
	FGameplayTag GetEquipmentSlot() const;
};
