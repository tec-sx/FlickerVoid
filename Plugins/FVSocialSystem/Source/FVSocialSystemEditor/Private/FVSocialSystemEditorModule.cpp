#include "FVCoreEditor.h"
#include "FVSocialTypes.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FVSocialSystemEditor"

static const FName SocialMenuSection("SocialSystem");

class FFVSocialSystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
		{
			FFVCoreEditorModule::AddSettingsMenuAction(SocialMenuSection, LOCTEXT("SocialSection", "Social System"),
				"GlobalSettings", LOCTEXT("GlobalSettings", "Global Settings"), UFVSocialSettings::StaticClass());
		}));
	}

	virtual void ShutdownModule() override
	{
		FFVCoreEditorModule::RemoveMenuSection(SocialMenuSection);
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVSocialSystemEditorModule, FVSocialSystemEditor)
