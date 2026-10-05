#include "FVAttributeSettings.h"
#include "FVCoreEditor.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FVAttributeSystemEditor"

static const FName AttributeMenuSection("AttributeSystem");

class FFVAttributeSystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
		{
			const FText SectionLabel = LOCTEXT("AttributeSection", "Attribute System");
			FFVCoreEditorModule::AddSettingsMenuAction(AttributeMenuSection, SectionLabel, "GlobalSettings",
				LOCTEXT("GlobalSettings", "Global Settings"), UFVAttributeSettings::StaticClass());
			FFVCoreEditorModule::AddAssetMenuAction(AttributeMenuSection, SectionLabel, "DefaultSet",
				LOCTEXT("DefaultSet", "Default Attribute Set"), []() -> UObject*
				{
					UObject* Asset = UFVAttributeSettings::Get().DefaultAttributeSet.LoadSynchronous();
					if (Asset == nullptr)
					{
						FFVCoreEditorModule::OpenSettings(UFVAttributeSettings::StaticClass());
					}
					return Asset;
				});
		}));
	}

	virtual void ShutdownModule() override
	{
		FFVCoreEditorModule::RemoveMenuSection(AttributeMenuSection);
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVAttributeSystemEditorModule, FVAttributeSystemEditor)
