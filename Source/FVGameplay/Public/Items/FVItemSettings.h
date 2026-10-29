#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVItemSettings.generated.h"

class AActor;
class UAnimMontage;
class UAnimSequenceBase;
class UFVInteractableDefinition;

/** Game-wide item setup: offers of world items, the world item actor, hand sockets and default item animations. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Items"))
class FLICKERVOIDGAMEPLAY_API UFVItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UFVItemSettings& Get() { return *GetDefault<UFVItemSettings>(); }

	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ForceInlineRow, Categories = "Item", ToolTip = "Offers of world items by item category; the most specific matching category wins. A World fragment's Interactable overrides it."))
	TMap<FGameplayTag, TSoftObjectPtr<UFVInteractableDefinition>> InteractablesByCategory;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ToolTip = "Offers of world items whose category has no entry."))
	TSoftObjectPtr<UFVInteractableDefinition> DefaultInteractable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "World", meta = (ToolTip = "Actor spawned for dropped or thrown items; the AngelScript AFVWorldItem or a Blueprint of it."))
	TSoftClassPtr<AActor> WorldItemClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "World", meta = (ClampMin = 0, Units = "cm", ToolTip = "How far in front of the character dropped items land."))
	float DropDistance = 60.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Hands", meta = (ToolTip = "Socket on the character mesh that right hand items attach to."))
	FName RightHandSocket = TEXT("hand_r_item");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Hands", meta = (ToolTip = "Socket on the character mesh that left hand items attach to."))
	FName LeftHandSocket = TEXT("hand_l_item");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ForceInlineRow, Categories = "Animation.Item", ToolTip = "Montages by style for characters whose Item Animations fragment has no entry."))
	TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> DefaultMontages;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ForceInlineRow, Categories = "Animation.Item.Hold", ToolTip = "Upper body poses by hold style for characters whose Item Animations fragment has no entry."))
	TMap<FGameplayTag, TSoftObjectPtr<UAnimSequenceBase>> DefaultHoldPoses;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = 0, Units = "cm", ToolTip = "Items below this height above the character's feet use Animation.Item.Pickup.Low."))
	float PickupLowHeight = 40.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = 0, Units = "cm", ToolTip = "Items above this height above the character's feet use Animation.Item.Pickup.High."))
	float PickupHighHeight = 140.f;
};
