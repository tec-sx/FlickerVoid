#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Data/FVDefinition.h"
#include "FVLootTableDefinition.generated.h"

class UFVItemDefinition;
class UFVLootTableDefinition;

/** One line of a loot table: an item or a nested table, with a quantity range, a chance and conditions. */
USTRUCT(BlueprintType)
struct FVINVENTORYSYSTEM_API FFVLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ToolTip = "Rolls this table instead of giving an item; nest tables to go past three entries."))
	TObjectPtr<UFVLootTableDefinition> Table;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 1, ToolTip = "Fewest units given, or times the nested table is rolled."))
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 1, ToolTip = "Most units given, or times the nested table is rolled."))
	int32 MaxQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "Probability the entry is given, from 0 to 1."))
	float Chance = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ToolTip = "Instigator is who loots, Target the looted actor or the receiver of a Give Loot effect."))
	FFVConditionSet Conditions;
};

/** Items and money a container, body or reward gives. Keep three entries at most and nest tables for more. */
UCLASS(BlueprintType)
class FVINVENTORYSYSTEM_API UFVLootTableDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot", meta = (TitleProperty = "Item"))
	TArray<FFVLootEntry> Entries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 0))
	int32 MinMoney = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 0))
	int32 MaxMoney = 0;
};
