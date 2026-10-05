#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "GameplayTagContainer.h"
#include "Items/FVItemDefinition.h"
#include "FVItemFragments.generated.h"

class USkeletalMesh;

/** Wearable in an equipment slot. Granted tags (e.g. Disguise.Faction.Police) are added to the wearer while equipped. */
USTRUCT(BlueprintType, meta = (DisplayName = "Equippable"))
struct FVINVENTORYSYSTEM_API FFVItemFragment_Equippable : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable", meta = (Categories = "Equipment.Slot"))
	FGameplayTag Slot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable")
	FGameplayTagContainer GrantedTags;

	/** Story gating, e.g. an outfit that can't be worn before a quest step. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable")
	FFVConditionSet EquipConditions;

	/** Applied by the game layer on equipment change. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equippable")
	TSoftObjectPtr<USkeletalMesh> Mesh;
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
