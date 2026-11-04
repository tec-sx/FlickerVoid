#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVInventorySettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Inventory"))
class FVINVENTORYSYSTEM_API UFVInventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UFVInventorySettings& Get() { return *GetDefault<UFVInventorySettings>(); }

	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	bool IsWeightEnforced() const;
	bool IsSpaceEnforced() const;

	/** Debug overrides of the limits, set by the Debug module's FVCvar.Inventory.Debug.Enforce* CVars: -1 uses the settings, 0 off, 1 on. */
	static int32 EnforceWeightOverride;
	static int32 EnforceSpaceOverride;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ToolTip = "Carried weight is limited. Override while debugging with FVCvar.Inventory.Debug.EnforceWeight."))
	bool bEnforceWeight = true;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ToolTip = "Items take up space from equipped containers. Override while debugging with FVCvar.Inventory.Debug.EnforceSpace."))
	bool bEnforceSpace = true;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = 0, Units = "kg", ToolTip = "Weight the character can carry with nothing equipped."))
	float BaseWeightLimit = 20.f;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = 0, ToolTip = "Cells available with no bag or backpack: pockets."))
	int32 BasePocketCells = 6;

	UPROPERTY(Config, EditAnywhere, Category = "Lists", meta = (Categories = "Item", ToolTip = "Item categories kept in the Keys and Quest list: they take no cells and no weight."))
	FGameplayTagContainer KeyItemCategories;
};
