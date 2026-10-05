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

	/** Carried weight is limited. Override at runtime with FVCvar.Inventory.EnforceWeight. */
	UPROPERTY(Config, EditAnywhere, Category = "Limits")
	bool bEnforceWeight = true;

	/** Items take up space from equipped containers. Override at runtime with FVCvar.Inventory.EnforceSpace. */
	UPROPERTY(Config, EditAnywhere, Category = "Limits")
	bool bEnforceSpace = true;

	/** Weight the character can carry with nothing equipped. */
	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = 0, Units = "kg"))
	float BaseWeightLimit = 20.f;

	/** Grid cells available with no bag or backpack: pockets. */
	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = 0))
	int32 BasePocketCells = 6;

	/** Width of the inventory grid in cells; item footprints are measured against it. */
	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = 1))
	int32 GridWidth = 6;
};
