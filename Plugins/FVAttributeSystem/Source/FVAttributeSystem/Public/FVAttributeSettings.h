#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "FVAttributeSettings.generated.h"

class UFVAttributeSetDefinition;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Attributes"))
class FVATTRIBUTESYSTEM_API UFVAttributeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UFVAttributeSettings& Get() { return *GetDefault<UFVAttributeSettings>(); }

	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	/** Applied to every attribute component that has no starting set of its own. */
	UPROPERTY(Config, EditAnywhere, Category = "Attributes")
	TSoftObjectPtr<UFVAttributeSetDefinition> DefaultAttributeSet;

	/** Where attribute definitions live, for the debug commands that look them up by name. */
	UPROPERTY(Config, EditAnywhere, Category = "Attributes", meta = (ContentDir))
	FDirectoryPath DefinitionDirectory;
};
