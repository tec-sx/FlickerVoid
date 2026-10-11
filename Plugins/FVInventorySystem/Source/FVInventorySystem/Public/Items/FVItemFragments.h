#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "GameplayTagContainer.h"
#include "Items/FVItemDefinition.h"
#include "FVItemFragments.generated.h"

class AActor;
class USkeletalMesh;
class UStaticMesh;

/** Wearable in one of its slots. Granted tags (e.g. Disguise.Faction.Police) are added to the wearer while equipped. */
USTRUCT(BlueprintType, meta = (DisplayName = "Equippable"))
struct FVINVENTORYSYSTEM_API FFVItemFragment_Equippable : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable", meta = (Categories = "Equipment.Slot", ToolTip = "Worn in any one of these, e.g. either arm for a bracelet."))
	FGameplayTagContainer Slots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable", meta = (Categories = "Equipment.Slot", ToolTip = "Covered while worn: whatever is worn there comes off and they stay empty."))
	FGameplayTagContainer BlockedSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable", meta = (ToolTip = "Story gating, e.g. an outfit that can't be worn before a quest step."))
	FFVConditionSet EquipConditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable", meta = (ToolTip = "Applied by the game layer on equipment change."))
	TSoftObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Moved into Slots on load."))
	FGameplayTag Slot_DEPRECATED;
};

/** A bag or backpack: adds carrying space and weight allowance while equipped. */
USTRUCT(BlueprintType, meta = (DisplayName = "Container"))
struct FVINVENTORYSYSTEM_API FFVItemFragment_Container : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (ClampMin = 0))
	int32 Cells = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (Units = "kg"))
	float WeightBonus = 5.f;
};

/** Can be used from the inventory. Instigator is the owner, Target is the owner too. */
USTRUCT(BlueprintType, meta = (DisplayName = "Usable"))
struct FVINVENTORYSYSTEM_API FFVItemFragment_Usable : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usable")
	FFVConditionSet UseConditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usable")
	FFVEffectList Effects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Usable")
	bool bConsumeOnUse = true;
};

/** How the item looks in the world and what spawns when it is dropped. */
USTRUCT(BlueprintType, meta = (DisplayName = "Pickup"))
struct FVINVENTORYSYSTEM_API FFVItemFragment_Pickup : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ToolTip = "Shown by FVItemPickup actors holding this item."))
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ToolTip = "Spawned when the item is dropped; empty uses Default Pickup Class from the inventory settings."))
	TSoftClassPtr<AActor> PickupClass;
};
