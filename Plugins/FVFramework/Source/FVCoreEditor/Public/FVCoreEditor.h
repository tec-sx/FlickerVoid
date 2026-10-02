#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Framework/Docking/TabManager.h"

class UDeveloperSettings;

class FVCOREEDITOR_API FFVCoreEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FFVCoreEditorModule& Get();

	void RegisterDebuggerTab(FName TabName, const FText& DisplayName, const FOnSpawnTab& SpawnDelegate);
	void UnregisterDebuggerTab(FName TabName);

	static void AddMenuAction(FName Section, const FText& SectionLabel, FName ActionName, const FText& Label, const FText& ToolTip, TFunction<void()> Action);
	static void AddSettingsMenuAction(FName Section, const FText& SectionLabel, FName ActionName, const FText& Label, TSubclassOf<UDeveloperSettings> SettingsClass);
	static void AddAssetMenuAction(FName Section, const FText& SectionLabel, FName ActionName, const FText& Label, TFunction<UObject*()> GetAsset);
	static void OpenSettings(TSubclassOf<UDeveloperSettings> SettingsClass);
	static void RemoveMenuSection(FName Section);

private:
	void RegisterMenus();
	TSharedPtr<FWorkspaceItem> DebuggerGroup;
	TArray<FName> DebuggerTabs;
};
