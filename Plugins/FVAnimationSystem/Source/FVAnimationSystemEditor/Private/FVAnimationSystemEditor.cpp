#include "FVAnimationSystemEditor.h"

#include "FVAnimationSettings.h"
#include "FVCoreEditor.h"
#include "MeatbagFactory/FVMeatbagFactoryTab.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FFVAnimationSystemEditorModule"

static const FName AnimationMenuSection("AnimationSystem");

void FFVAnimationSystemEditorModule::StartupModule()
{
	FVMeatbagFactoryTab::Register();
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
	{
		FFVCoreEditorModule::AddSettingsMenuAction(AnimationMenuSection, LOCTEXT("AnimationSection", "Animation System"), "GlobalSettings",
			LOCTEXT("GlobalSettings", "Global Settings"), UFVAnimationSettings::StaticClass());
	}));
}

void FFVAnimationSystemEditorModule::ShutdownModule()
{
	FVMeatbagFactoryTab::Unregister();
	FFVCoreEditorModule::RemoveMenuSection(AnimationMenuSection);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVAnimationSystemEditorModule, FVAnimationSystemEditor)
