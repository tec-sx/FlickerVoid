#include "FlickerVoidCoreEditor.h"

#include "SSettingsEditorCheckoutNotice.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Editor.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "FVCore/Public/FactDB/FVFactPreset.h"
#include "FVCore/Public/FactDB/FVFactSubsystem.h"
#include "FactDB/Debugger/FVFactDebugger.h"
#include "FactDB/Debugger/FVFactDebuggerStyle.h"
#include "Framework/Notifications/NotificationManager.h"
#include "FVCore/Public/Logging/FVLogCategories.h"
#include "FVCore/Public/Logging/FVLogSystem.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FFlickerVoidCoreEditorModule"

static const FName FactDebuggerTabName("FactDebugger");

static FAutoConsoleCommandWithWorld GShowFactDebugger
(
	TEXT("Facts.Debugger"),
	TEXT("Opens Fact Debugger window"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		FGlobalTabmanager::Get()->TryInvokeTab(FactDebuggerTabName);
	})
);


#define LOCTEXT_NAMESPACE "FactDebugger"

void FFlickerVoidCoreEditorModule::StartupModule()
{
	// Facts Debugger
	FFVFactDebuggerStyle::Register();
	FTabSpawnerEntry& Tab = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		                                                FactDebuggerTabName,
		                                                FOnSpawnTab::CreateRaw(
			                                                this, &FFlickerVoidCoreEditorModule::SpawnFactDebuggerTab))
	                                                .SetDisplayName(LOCTEXT("FactDebugger_Title", "Fact Debugger"))
	                                                .SetTooltipText(
		                                                LOCTEXT("FactDebugger_ToolTip", "Open Fact Debugger tab."))
	                                                .SetIcon(FSlateIcon(FFVFactDebuggerStyle::GetStyleSetName(),
	                                                                    "ClassIcon.FactPreset"));

	Tab.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory());

	GameInstanceStartedHandle = FWorldDelegates::OnStartGameInstance.AddRaw(
		this, &FFlickerVoidCoreEditorModule::HandleGameInstanceStarted);
#if WITH_EDITOR
	GameInstanceEndedHandle = FEditorDelegates::EndPIE.AddLambda([ this ](const bool)
	{
		HandleGameInstanceEnded();
	});
#endif

	// Register to get a warning on startup if settings aren't configured correctly
	UAssetManager::CallOrRegister_OnAssetManagerCreated(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFlickerVoidCoreEditorModule::HandleAssetManagerCreated));
}

void FFlickerVoidCoreEditorModule::ShutdownModule()
{
	FFVFactDebuggerStyle::Unregister();

	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartedHandle);
	GameInstanceStartedHandle.Reset();
#if WITH_EDITOR
	FEditorDelegates::EndPIE.Remove(GameInstanceEndedHandle);
	GameInstanceEndedHandle.Reset();
#endif

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FactDebuggerTabName);

		if (FactDebuggerTab.IsValid())
		{
			FactDebuggerTab.Pin()->RequestCloseTab();
		}
	}
}

bool FFlickerVoidCoreEditorModule::IsGameInstanceStarted() const
{
	return WeakGameInstance.IsValid();
}

void FFlickerVoidCoreEditorModule::LoadFactPreset(const UFVFactPreset* InPreset) const
{
	if (WeakGameInstance.IsValid())
	{
		if (WeakGameInstance.Pin()->GetWorld() == nullptr)
		{
			FV_LOG_ERROR(LogFVCoreEditor, "%hs: WorldContextObject is null", __FUNCTION__);
			return;
		}

		if (InPreset == nullptr)
		{
			FV_LOG_ERROR(LogFVCoreEditor, "%hs: Preset is null", __FUNCTION__);
			return;
		}

		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(WeakGameInstance.Pin()->GetWorld());
		for (auto [Tag, Value] : InPreset->PresetValues)
		{
			FactSubsystem.ChangeFactValue(Tag, Value, EFVFactValueChangeType::Set);
		}
	}
}

void FFlickerVoidCoreEditorModule::LoadFactPresets(const TArray<UFVFactPreset*>& InPresets) const
{
	if (WeakGameInstance.IsValid())
	{
		for (const UFVFactPreset* Preset : InPresets)
		{
			if (Preset == nullptr)
			{
				FV_LOG_ERROR(LogFVCoreEditor, "%hs: Null preset in TArray", __FUNCTION__);
				continue;
			}

			LoadFactPreset(Preset);
		}
	}
}

UFVFactSubsystem* FFlickerVoidCoreEditorModule::TryGetFactSubsystem() const
{
	if (WeakGameInstance.IsValid())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(WeakGameInstance->GetWorld());
		return &FactSubsystem;
	}

	return nullptr;
}

void FFlickerVoidCoreEditorModule::HandleGameInstanceStarted(UGameInstance* GameInstance)
{
	WeakGameInstance = GameInstance;
	(void)OnGameInstanceStarted.ExecuteIfBound();
}

void FFlickerVoidCoreEditorModule::HandleGameInstanceEnded()
{
	WeakGameInstance = nullptr;
	(void)OnGameInstanceEnded.ExecuteIfBound();
}

void FFlickerVoidCoreEditorModule::HandleAssetManagerCreated()
{
	// Make sure the game has the appropriate asset manager configuration or we won't be able to load game feature data assets
	FPrimaryAssetId DummyGameFeatureDataAssetId(UFVFactPreset::StaticClass()->GetFName(), NAME_None);
	FPrimaryAssetRules GameDataRules = UAssetManager::Get().GetPrimaryAssetRules(DummyGameFeatureDataAssetId);
	if (FApp::HasProjectName() && GameDataRules.IsDefault())
	{
		FMessageLog("LoadErrors").Error()
		                         ->AddToken(FTextToken::Create(FText::Format(
			                         LOCTEXT("MissingRuleForFactPreset",
			                                 "Asset Manager settings do not include an entry for assets of type {0}, which is required for FactDebugger's \"Presets\" picker to show assets in non-editor builds.\n"),
			                         FText::FromName(UFVFactPreset::StaticClass()->GetFName()))))
		                         ->AddToken(FActionToken::Create(
			                         LOCTEXT("AddRuleForFactPreset", "Add entry to PrimaryAssetTypesToScan?"), FText(),
			                         FOnActionTokenExecuted::CreateRaw(
				                         this, &FFlickerVoidCoreEditorModule::AddDefaultGameDataRule), true));
	}
}

void FFlickerVoidCoreEditorModule::AddDefaultGameDataRule()
{
	// Check out the ini or make it writable
	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();
	const FString& ConfigFileName = Settings->GetDefaultConfigFilename();
	bool bSuccess = false;

	FText NotificationOpText;
	if (SettingsHelpers::IsCheckedOut(ConfigFileName, true) == false)
	{
		FText ErrorMessage;
		bSuccess = SettingsHelpers::CheckOutOrAddFile(ConfigFileName, true, IsRunningCommandlet() == false,
		                                              &ErrorMessage);
		if (bSuccess)
		{
			NotificationOpText = LOCTEXT("CheckedOutAssetManagerIni", "Rule for {0} added.\nChecked out {1}");
		}
		else
		{
			FV_LOG_ERROR(LogFVCoreEditor, "%s", *ErrorMessage.ToString());
			bSuccess = SettingsHelpers::MakeWritable(ConfigFileName);

			if (bSuccess)
			{
				NotificationOpText = LOCTEXT("MadeWritableAssetManagerIni",
				                             "Rule for {0} added.\nMade {1} writable (you may need to manually add to revision control)");
			}
			else
			{
				NotificationOpText = LOCTEXT("FailedToTouchAssetManagerIni",
				                             "No rule for {0} was added.\nFailed to check out {1} or make it writable");
			}
		}
	}
	else
	{
		NotificationOpText = LOCTEXT("UpdatedAssetManagerIni", "Rule for {0} added.\nUpdated {1}");
		bSuccess = true;
	}

	// Add the rule to project settings
	if (bSuccess)
	{
		FPrimaryAssetTypeInfo NewTypeInfo(
			UFVFactPreset::StaticClass()->GetFName(),
			UFVFactPreset::StaticClass(),
			false,
			false,
			{{"/Game"}},
			{}
		);
		NewTypeInfo.Rules.CookRule = EPrimaryAssetCookRule::DevelopmentAlwaysProductionUnknownCook;

		Settings->Modify(true);

		Settings->PrimaryAssetTypesToScan.Add(NewTypeInfo);

		Settings->PostEditChange();
		Settings->TryUpdateDefaultConfigFile();

		UAssetManager::Get().ReinitializeFromConfig();
	}

	// Show a message that the file was checked out/updated and must be submitted
	FNotificationInfo Info(FText::Format(NotificationOpText,
	                                     FText::FromString(UFVFactPreset::StaticClass()->GetName()),
	                                     FText::FromString(FPaths::GetCleanFilename(ConfigFileName))
	));
	Info.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(Info);
}

TSharedRef<SDockTab> FFlickerVoidCoreEditorModule::SpawnFactDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SAssignNew(FactDebuggerTab, SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SummonFactDebuggerUI().ToSharedRef()
		];
}

TSharedPtr<SWidget> FFlickerVoidCoreEditorModule::SummonFactDebuggerUI()
{
	if (IsInGameThread())
	{
		return SAssignNew(FactDebugger, SFVFactDebugger).bIsGameStarted(WeakGameInstance.IsValid());
	}

	return {};
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFlickerVoidCoreEditorModule, FlickerVoidCoreEditor)
