#include "FVInteractionSystemEditor.h"

#include "BlueprintCompilationManager.h"
#include "Data/FVInteractionUISettings.h"
#include "Engine/Blueprint.h"
#include "FVCoreEditor.h"
#include "FVInteractionSystemSettings.h"
#include "ToolMenus.h"
#include "UObject/Package.h"
#include "Validation/InteractionBlueprintCompilerExtension.h"
#include "Validation/InteractionCompileRuleRegistry.h"

#define LOCTEXT_NAMESPACE "FFVInteractionSystemEditorModule"

static const FName InteractionMenuSection("InteractionSystem");

void FFVInteractionSystemEditorModule::StartupModule()
{
	FInteractionCompileRuleRegistry::Get().RegisterDefaultRules();
	RegisterCompilerExtension();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
	{
		const FText SectionLabel = LOCTEXT("InteractionSection", "Interaction System");
		FFVCoreEditorModule::AddSettingsMenuAction(InteractionMenuSection, SectionLabel, "GlobalSettings",
			LOCTEXT("GlobalSettings", "Global Settings"), UFVInteractionSystemSettings::StaticClass());
		FFVCoreEditorModule::AddAssetMenuAction(InteractionMenuSection, SectionLabel, "UISettings",
			LOCTEXT("UISettings", "UI Settings"), []() -> UObject*
			{
				UObject* Asset = UFVInteractionSystemSettings::Get().InteractionUISettings.LoadSynchronous();
				if (!Asset)
				{
					FFVCoreEditorModule::OpenSettings(UFVInteractionSystemSettings::StaticClass());
				}
				return Asset;
			});
	}));
}

void FFVInteractionSystemEditorModule::ShutdownModule()
{
	FFVCoreEditorModule::RemoveMenuSection(InteractionMenuSection);
	UnregisterCompilerExtension();
	FInteractionCompileRuleRegistry::Get().Reset();
}

void FFVInteractionSystemEditorModule::RegisterCompilerExtension()
{
	CompilerExtension = NewObject<UInteractionBlueprintCompilerExtension>(GetTransientPackage());
	CompilerExtension->AddToRoot();

	FBlueprintCompilationManager::RegisterCompilerExtension(UBlueprint::StaticClass(), CompilerExtension);
}

void FFVInteractionSystemEditorModule::UnregisterCompilerExtension()
{
	if (CompilerExtension && !GExitPurge)
	{
		CompilerExtension->RemoveFromRoot();
	}

	CompilerExtension = nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVInteractionSystemEditorModule, FVInteractionSystemEditor)
