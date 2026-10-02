#include "FVCoreEditor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Customizations/FVConditionSetCustomization.h"
#include "Data/FVDefinition.h"
#include "Editor.h"
#include "EditorValidatorSubsystem.h"
#include "PropertyEditorModule.h"
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
}

void FFVCoreEditorModule::ShutdownModule()
{
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
