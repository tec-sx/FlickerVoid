#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Framework/Docking/TabManager.h"

class FVCOREEDITOR_API FFVCoreEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FFVCoreEditorModule& Get();

	void RegisterDebuggerTab(FName TabName, const FText& DisplayName, const FOnSpawnTab& SpawnDelegate);
	void UnregisterDebuggerTab(FName TabName);

private:
	TSharedPtr<FWorkspaceItem> DebuggerGroup;
	TArray<FName> DebuggerTabs;
};
