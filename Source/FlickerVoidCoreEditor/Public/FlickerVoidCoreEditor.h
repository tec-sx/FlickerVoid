#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/GameInstance.h"

class SFVFactDebugger;
class UFVFactSubsystem;
class UFVFactPreset;

class FLICKERVOIDCOREEDITOR_API FFlickerVoidCoreEditorModule : public IModuleInterface
{
public:
	static FORCEINLINE FFlickerVoidCoreEditorModule& Get()
	{
		static FName FlickerVoidCoreEditorModule("FlickerVoidEditorModule");
		return FModuleManager::LoadModuleChecked<FFlickerVoidCoreEditorModule>(FlickerVoidCoreEditorModule);
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool IsGameInstanceStarted() const;

	DECLARE_DELEGATE(FGameInstanceStateChanged)
	FGameInstanceStateChanged OnGameInstanceStarted;
	FGameInstanceStateChanged OnGameInstanceEnded;

	// FACT
	void LoadFactPreset(const UFVFactPreset* InPreset) const;
	void LoadFactPresets(const TArray<UFVFactPreset*>& InPresets) const;
	UFVFactSubsystem* TryGetFactSubsystem() const;

private:
	void HandleGameInstanceStarted(UGameInstance* GameInstance);
	void HandleGameInstanceEnded();

	void HandleAssetManagerCreated();
	void AddDefaultGameDataRule();

	TWeakObjectPtr<UGameInstance> WeakGameInstance;

	// FACT
	TSharedRef<SDockTab> SpawnFactDebuggerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<SWidget> SummonFactDebuggerUI();

	TWeakPtr<SDockTab> FactDebuggerTab;
	TWeakPtr<SFVFactDebugger> FactDebugger;

};
