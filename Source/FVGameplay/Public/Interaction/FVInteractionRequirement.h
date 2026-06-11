#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "FVInteractionRequirement.generated.h"

//~=============================================================================
// Abstract base — subclass in C++ or AngelScript/Blueprint
//~=============================================================================

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, CollapseCategories)
class FLICKERVOIDGAMEPLAY_API UFVInteractionRequirement : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Requirement")
	bool IsMet(AActor* Instigator) const;
	virtual bool IsMet_Implementation(AActor* Instigator) const { return true; }

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Requirement")
	FText GetUnmetReason(AActor* Instigator) const;
	virtual FText GetUnmetReason_Implementation(AActor* Instigator) const { return FText::GetEmpty(); }
};

//~=============================================================================
// Built-in requirements
//~=============================================================================

UCLASS(DisplayName = "Tag Requirement", meta = (DisplayThumbnail = "false"))
class FLICKERVOIDGAMEPLAY_API UFVTagRequirement : public UFVInteractionRequirement
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FGameplayTag RequiredTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FText UnmetReason = NSLOCTEXT("FVInteraction", "TagReqUnmet", "Missing required skill or status.");

	virtual bool IsMet_Implementation(AActor* Instigator) const override;
	virtual FText GetUnmetReason_Implementation(AActor* Instigator) const override { return UnmetReason; }
};

// Instigator must have a minimum GAS attribute value.
// Designer selects the attribute from the editor picker (e.g. FVCharacterAttributeSet.Strength).
UCLASS(DisplayName = "Attribute Requirement", meta = (DisplayThumbnail = "false"))
class FLICKERVOIDGAMEPLAY_API UFVAttributeRequirement : public UFVInteractionRequirement
{
	GENERATED_BODY()

public:
	// Pick directly in the editor — no tag-to-attribute mapping needed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FGameplayAttribute RequiredAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement", meta = (ClampMin = "0"))
	float MinimumValue = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FText UnmetReason;

	virtual bool IsMet_Implementation(AActor* Instigator) const override;
	virtual FText GetUnmetReason_Implementation(AActor* Instigator) const override { return UnmetReason; }
};

// Instigator must have an item with the specified tag in inventory
// Delegated to Blueprint/AngelScript because FVItems is a sibling module
UCLASS(Abstract, DisplayName = "Item Requirement (implement in BP)", meta = (DisplayThumbnail = "false"))
class FLICKERVOIDGAMEPLAY_API UFVItemRequirement : public UFVInteractionRequirement
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement", meta = (Categories = "Item"))
	FGameplayTag RequiredItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FText UnmetReason;

	virtual FText GetUnmetReason_Implementation(AActor* Instigator) const override { return UnmetReason; }
	// IsMet_Implementation left for Blueprint override — needs FVItems inventory access
};
