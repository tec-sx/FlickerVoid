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
	template <typename T>
	const T* FindFragment() const { return FVFragments::Find<T>(Fragments); }

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsEquippable() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsUsable() const;

	UFUNCTION(BlueprintPure, Category = "Item", meta = (ToolTip = "GridSize.X * GridSize.Y."))
	int32 GetCellCount() const { return FMath::Max(1, GridSize.X) * FMath::Max(1, GridSize.Y); }

	UFUNCTION(BlueprintPure, Category = "Item", meta = (ToolTip = "Slots the item can be worn in (any one of them); empty if not equippable."))
	FGameplayTagContainer GetEquipmentSlots() const;

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (Categories = "Item"))
	FGameplayTag Category;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 0, Units = "kg", ToolTip = "Weight of a single unit."))
	float Weight = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1, ToolTip = "Footprint in cells (Width x Height); each stack claims it once however many units it holds."))
	FIntPoint GridSize = FIntPoint(1, 1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 0, ToolTip = "Units one stack can hold; more units start new stacks. 1 = every unit is its own entry, 0 = unlimited."))
	int32 MaxQuantity = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Quest items can't be dropped or given away by the player."))
	bool bQuestItem = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVItemFragment>> Fragments;
};
