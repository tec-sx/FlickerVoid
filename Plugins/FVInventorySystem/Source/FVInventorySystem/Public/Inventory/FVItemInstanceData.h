#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/FVInventoryComponent.h"
#include "FVItemInstanceData.generated.h"

/** Uses left on a partly used unit: the stack's next one. */
USTRUCT(BlueprintType, meta = (DisplayName = "Charges Left"))
struct FVINVENTORYSYSTEM_API FFVItemInstance_Charges : public FFVItemInstanceFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Charges", meta = (ClampMin = 0, ToolTip = "Uses left on the unit; 0 = as many as the item's Charges fragment."))
	int32 Remaining = 0;
};

/** Taken without the owner's consent. Owner is a faction or character Id tag; empty = unknown owner. */
USTRUCT(BlueprintType, meta = (DisplayName = "Stolen"))
struct FVINVENTORYSYSTEM_API FFVItemInstance_Stolen : public FFVItemInstanceFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Stolen", meta = (ToolTip = "Faction or character Id the item was taken from; empty = unknown owner."))
	FGameplayTag Owner;
};
