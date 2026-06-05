// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/FVItemDataAsset.h"
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemDataAsset)

void UFVItemDataAsset::OnItemUsed_Implementation(AActor* User)
{
	// Default implementation - override in Blueprint or C++ child classes
	// This is called when an item action with bConsumesItem is executed
}

void UFVItemDataAsset::OnItemInspected_Implementation(AActor* Inspector)
{
	// Default implementation - override in Blueprint or C++ child classes
	// This is called when the player inspects/examines the item
}

FGameplayTag UFVItemDataAsset::GetPrimaryCategoryTag() const
{
	return ItemData.GetPrimaryCategoryTag();
}

FGameplayTag UFVItemDataAsset::GetRarityTag() const
{
	return ItemData.GetRarityTag();
}

bool UFVItemDataAsset::IsInCategory(const FGameplayTag& CategoryTag) const
{
	return ItemData.IsInCategory(CategoryTag);
}

bool UFVItemDataAsset::HasItemTag(const FGameplayTag& Tag) const
{
	return ItemData.ItemTags.HasTag(Tag);
}

bool UFVItemDataAsset::FindActionByTag(FGameplayTag ActionTag, FFVItemAction& OutAction) const
{
	const FFVItemAction* FoundAction = ItemData.AvailableActions.FindByPredicate(
		[&ActionTag](const FFVItemAction& Action)
		{
			return Action.ActionTag == ActionTag;
		}
	);

	if (FoundAction)
	{
		OutAction = *FoundAction;
		return true;
	}

	return false;
}

bool UFVItemDataAsset::HasAction(FGameplayTag ActionTag) const
{
	return ItemData.AvailableActions.ContainsByPredicate(
		[&ActionTag](const FFVItemAction& Action)
		{
			return Action.ActionTag == ActionTag;
		}
	);
}

#if WITH_EDITOR
void UFVItemDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Validate that ItemID is set
	if (ItemData.ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDataAsset '%s': ItemID is not set!"), *GetName());
	}

	// Validate that at least one category tag exists
	FGameplayTag CategoryParent = FGameplayTag::RequestGameplayTag(FName("Item.Category"), false);
	if (CategoryParent.IsValid())
	{
		bool bHasCategory = false;
		for (const FGameplayTag& Tag : ItemData.ItemTags)
		{
			if (Tag.MatchesTag(CategoryParent))
			{
				bHasCategory = true;
				break;
			}
		}

		if (!bHasCategory)
		{
			UE_LOG(LogTemp, Warning, TEXT("ItemDataAsset '%s': No category tag (Item.Category.*) assigned!"), *GetName());
		}
	}

	// Validate stack size
	if (ItemData.MaxStackSize < 1)
	{
		ItemData.MaxStackSize = 1;
		UE_LOG(LogTemp, Warning, TEXT("ItemDataAsset '%s': MaxStackSize cannot be less than 1. Reset to 1."), *GetName());
	}
}

FString UFVItemDataAsset::GetDesc()
{
	FString Description = ItemData.ItemName.ToString();
	
	// Add category
	FGameplayTag CategoryTag = GetPrimaryCategoryTag();
	if (CategoryTag.IsValid())
	{
		Description += FString::Printf(TEXT(" | %s"), *CategoryTag.ToString());
	}

	// Add rarity
	FGameplayTag RarityTag = GetRarityTag();
	if (RarityTag.IsValid())
	{
		Description += FString::Printf(TEXT(" | %s"), *RarityTag.ToString());
	}

	// Add value
	if (ItemData.Value > 0)
	{
		Description += FString::Printf(TEXT(" | Value: %d"), ItemData.Value);
	}

	return Description;
}
#endif
