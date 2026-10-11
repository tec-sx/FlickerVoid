#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "FVInventorySettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Inventory"))
class FVINVENTORYSYSTEM_API UFVInventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UFVInventorySettings& Get() { return *GetDefault<UFVInventorySettings>(); }

	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ToolTip = "Carried weight is limited. Override at runtime with FVCvar.Inventory.EnforceWeight."))
	bool bEnforceWeight = true;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ToolTip = "Items take up space from equipped containers. Override at runtime with FVCvar.Inventory.EnforceSpace."))
	bool bEnforceSpace = true;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = 0, Units = "kg", ToolTip = "Weight the character can carry with nothing equipped."))
	float BaseWeightLimit = 20.f;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = 0, ToolTip = "Cells available with no bag or backpack: pockets."))
	int32 BasePocketCells = 6;

	UPROPERTY(Config, EditAnywhere, Category = "Dropping", meta = (ToolTip = "Actor spawned for a dropped item whose Pickup fragment names no class. Needs an FV Pickup Component, e.g. a Blueprint of FVItemPickup."))
	TSoftClassPtr<AActor> DefaultPickupClass;

	UPROPERTY(Config, EditAnywhere, Category = "Dropping", meta = (ClampMin = 0, Units = "cm", ToolTip = "How far in front of the dropping actor the item lands."))
	float DropDistance = 80.f;
};
