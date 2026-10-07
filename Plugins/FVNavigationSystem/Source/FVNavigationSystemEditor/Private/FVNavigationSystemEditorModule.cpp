#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "FVCoreEditor.h"
#include "FVMapCapture.h"
#include "FVMapCaptureActor.h"
#include "FVNavigationTypes.h"
#include "Modules/ModuleManager.h"
#include "SFVMapCapturePanel.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FVNavigationSystemEditor"

static const FName NavigationMenuSection("NavigationSystem");
static const FName MapCaptureTab("FVMapCapture");

class FFVNavigationSystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MapCaptureTab, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
			{
				return SNew(SDockTab).TabRole(ETabRole::NomadTab)
					[
						SNew(SFVMapCapturePanel)
					];
			}))
			.SetDisplayName(LOCTEXT("MapCaptureTab", "Map Capture"))
			.SetTooltipText(LOCTEXT("MapCaptureTabTip", "Capture map layer images from map capture actors."))
			.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
			.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
		{
			const FText SectionLabel = LOCTEXT("NavigationSection", "Navigation System");
			FFVCoreEditorModule::AddSettingsMenuAction(NavigationMenuSection, SectionLabel,
				"GlobalSettings", LOCTEXT("GlobalSettings", "Global Settings"), UFVNavigationSettings::StaticClass());
			FFVCoreEditorModule::AddMenuAction(NavigationMenuSection, SectionLabel,
				"MapCapture", LOCTEXT("MapCapture", "Map Capture"), LOCTEXT("MapCaptureTip", "Open the map capture panel."),
				[] { FGlobalTabmanager::Get()->TryInvokeTab(MapCaptureTab); });
		}));

		CaptureRequestedHandle = AFVMapCaptureActor::OnCaptureRequested.AddLambda([](AFVMapCaptureActor* Actor)
		{
			FVMapCapture::CaptureAndNotify(Actor);
		});
	}

	virtual void ShutdownModule() override
	{
		AFVMapCaptureActor::OnCaptureRequested.Remove(CaptureRequestedHandle);
		FFVCoreEditorModule::RemoveMenuSection(NavigationMenuSection);

		if (FSlateApplication::IsInitialized())
		{
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MapCaptureTab);
		}
	}

private:
	FDelegateHandle CaptureRequestedHandle;
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVNavigationSystemEditorModule, FVNavigationSystemEditor)
