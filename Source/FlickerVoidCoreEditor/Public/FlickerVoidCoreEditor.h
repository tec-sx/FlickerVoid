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
	static FORCEINLINE FName GetModuleName()
	{
		static const FName ModuleName("FlickerVoidCoreEditor");
		return ModuleName;
	}

	static FORCEINLINE FFlickerVoidCoreEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FFlickerVoidCoreEditorModule>(GetModuleName());
	}

	// Shutdown-safe accessor. Returns null if the module is not loaded, never asserts.
	static FORCEINLINE FFlickerVoidCoreEditorModule* GetPtr()
	{
		return FModuleManager::GetModulePtr<FFlickerVoidCoreEditorModule>(GetModuleName());
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

	FDelegateHandle GameInstanceStartedHandle;
	FDelegateHandle GameInstanceEndedHandle;

	// FACT
	TSharedRef<SDockTab> SpawnFactDebuggerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<SWidget> SummonFactDebuggerUI();

	TWeakPtr<SDockTab> FactDebuggerTab;
	TWeakPtr<SFVFactDebugger> FactDebugger;

};
