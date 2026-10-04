#include "FVCoreEditor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Customizations/FVConditionSetCustomization.h"
#include "Data/FVDefinition.h"
#include "Editor.h"
#include "FVCoreNames.h"
#include "EditorValidatorSubsystem.h"
#include "Engine/DeveloperSettings.h"
#include "ISettingsModule.h"
#include "Layout/FVUILayout.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Time/FVWorldClock.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FVCoreEditor"

static const FName ConditionSetStructName("FVConditionSet");


static void ValidateAllDefinitions()
{
	const IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetData> Assets;
	Registry.GetAssetsByClass(UFVDefinition::StaticClass()->GetClassPathName(), Assets, true);

	UEditorValidatorSubsystem* Validator = GEditor ? GEditor->GetEditorSubsystem<UEditorValidatorSubsystem>() : nullptr;
	if (!Validator)
	{
		return;
	}

	FValidateAssetsSettings Settings;
	Settings.bShowIfNoFailures = true;
	FValidateAssetsResults Results;
	Validator->ValidateAssetsWithSettings(Assets, Settings, Results);
}

static FAutoConsoleCommand GValidateAllDefinitions(
	TEXT("FV.ValidateAll"),
	TEXT("Validates every FlickerVoid definition asset."),
	FConsoleCommandDelegate::CreateStatic(&ValidateAllDefinitions));

void FFVCoreEditorModule::StartupModule()
{
	DebuggerGroup = WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory()->AddGroup(
		LOCTEXT("DebuggerGroup", "FlickerVoid"));

	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditor.RegisterCustomPropertyTypeLayout(ConditionSetStructName,
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFVConditionSetCustomization::MakeInstance));

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFVCoreEditorModule::RegisterMenus));
}

void FFVCoreEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* MainMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
	MainMenu->AddSubMenu("MainMenu", NAME_None, "FlickerVoid", LOCTEXT("FlickerVoidMenu", "FlickerVoid"), LOCTEXT("FlickerVoidMenuTip", "FlickerVoid framework and plugin settings"));
	UToolMenus::Get()->RegisterMenu(FV::Names::EditorMenu);

	UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& ToolbarSection = Toolbar->FindOrAddSection("FlickerVoid");
	ToolbarSection.AddEntry(FToolMenuEntry::InitComboButton(
		"FlickerVoidToolbar",
		FUIAction(),
		FOnGetContent::CreateLambda([]
		{
			return UToolMenus::Get()->GenerateWidget(FV::Names::EditorMenu, FToolMenuContext());
		}),
		LOCTEXT("FlickerVoidToolbar", "FlickerVoid"),
		LOCTEXT("FlickerVoidToolbarTip", "FlickerVoid framework and plugin settings"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.GameSettings")));

	const FText FrameworkLabel = LOCTEXT("FrameworkSection", "Framework");
	AddMenuAction("Framework", FrameworkLabel, "UISettings", LOCTEXT("FrameworkUI", "UI Settings"), FText::GetEmpty(),
		[] { OpenSettings(UFVUISettings::StaticClass()); });
	AddMenuAction("Framework", FrameworkLabel, "WorldClockSettings", LOCTEXT("FrameworkClock", "World Clock Settings"), FText::GetEmpty(),
		[] { OpenSettings(UFVWorldClockSettings::StaticClass()); });
}

void FFVCoreEditorModule::AddMenuAction(FName Section, const FText& SectionLabel, FName ActionName, const FText& Label, const FText& ToolTip, TFunction<void()> Action)
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(FV::Names::EditorMenu);
	FToolMenuSection& MenuSection = Menu->FindOrAddSection(Section, SectionLabel);
	MenuSection.AddMenuEntry(ActionName, Label, ToolTip, FSlateIcon(), FUIAction(FExecuteAction::CreateLambda(MoveTemp(Action))));
}

void FFVCoreEditorModule::AddSettingsMenuAction(FName Section, const FText& SectionLabel, FName ActionName, const FText& Label, TSubclassOf<UDeveloperSettings> SettingsClass)
{
	AddMenuAction(Section, SectionLabel, ActionName, Label, FText::GetEmpty(), [SettingsClass] { OpenSettings(SettingsClass); });
}

void FFVCoreEditorModule::AddAssetMenuAction(FName Section, const FText& SectionLabel, FName ActionName, const FText& Label, TFunction<UObject*()> GetAsset)
{
	AddMenuAction(Section, SectionLabel, ActionName, Label, FText::GetEmpty(), [GetAsset = MoveTemp(GetAsset)]
	{
		UObject* Asset = GetAsset();
		if (Asset && GEditor)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Asset);
		}
	});
}

void FFVCoreEditorModule::OpenSettings(TSubclassOf<UDeveloperSettings> SettingsClass)
{
	const UDeveloperSettings* Settings = SettingsClass ? GetDefault<UDeveloperSettings>(SettingsClass) : nullptr;
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"); Settings && SettingsModule)
	{
		SettingsModule->ShowViewer(Settings->GetContainerName(), Settings->GetCategoryName(), Settings->GetSectionName());
	}
}

void FFVCoreEditorModule::RemoveMenuSection(FName Section)
{
	if (UToolMenus::IsToolMenuUIEnabled() && UObjectInitialized())
	{
		if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(FV::Names::EditorMenu))
		{
			Menu->RemoveSection(Section);
		}
	}
}

void FFVCoreEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	if (FSlateApplication::IsInitialized())
	{
		for (const FName TabName : DebuggerTabs)
		{
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
		}
	}
	DebuggerTabs.Reset();

	if (FPropertyEditorModule* PropertyEditor = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyEditor->UnregisterCustomPropertyTypeLayout(ConditionSetStructName);
	}
}

FFVCoreEditorModule& FFVCoreEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FFVCoreEditorModule>("FVCoreEditor");
}

void FFVCoreEditorModule::RegisterDebuggerTab(FName TabName, const FText& DisplayName, const FOnSpawnTab& SpawnDelegate)
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, SpawnDelegate)
		.SetDisplayName(DisplayName)
		.SetGroup(DebuggerGroup.ToSharedRef());
	DebuggerTabs.AddUnique(TabName);
}

void FFVCoreEditorModule::UnregisterDebuggerTab(FName TabName)
{
	if (DebuggerTabs.Remove(TabName) > 0 && FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVCoreEditorModule, FVCoreEditor)
