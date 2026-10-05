#include "FVCoreEditor.h"
#include "FVDialogueTypes.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FVDialogueSystemEditor"

static const FName DialogueMenuSection("DialogueSystem");

class FFVDialogueSystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
		{
			FFVCoreEditorModule::AddSettingsMenuAction(DialogueMenuSection, LOCTEXT("DialogueSection", "Dialogue System"),
				"GlobalSettings", LOCTEXT("GlobalSettings", "Global Settings"), UFVDialogueSettings::StaticClass());
		}));
	}

	virtual void ShutdownModule() override
	{
		FFVCoreEditorModule::RemoveMenuSection(DialogueMenuSection);
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVDialogueSystemEditorModule, FVDialogueSystemEditor)
