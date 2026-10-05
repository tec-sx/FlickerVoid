#include "FVCoreEditor.h"
#include "FVInventorySettings.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FVInventorySystemEditor"

static const FName InventoryMenuSection("InventorySystem");

class FFVInventorySystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
		{
			FFVCoreEditorModule::AddSettingsMenuAction(InventoryMenuSection, LOCTEXT("InventorySection", "Inventory System"),
				"GlobalSettings", LOCTEXT("GlobalSettings", "Global Settings"), UFVInventorySettings::StaticClass());
		}));
	}

	virtual void ShutdownModule() override
	{
		FFVCoreEditorModule::RemoveMenuSection(InventoryMenuSection);
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVInventorySystemEditorModule, FVInventorySystemEditor)
