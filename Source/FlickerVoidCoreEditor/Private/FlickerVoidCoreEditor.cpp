#include "FlickerVoidCoreEditor.h"

#include "Editor.h"

void FFlickerVoidCoreEditorModule::StartupModule()
{
	GameInstanceStartedHandle = FWorldDelegates::OnStartGameInstance.AddRaw(this, &FFlickerVoidCoreEditorModule::HandleGameInstanceStarted);
	GameInstanceEndedHandle = FEditorDelegates::EndPIE.AddLambda([this](const bool)
		{
			HandleGameInstanceEnded();
		});
}

void FFlickerVoidCoreEditorModule::ShutdownModule()
{
	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartedHandle);
	GameInstanceStartedHandle.Reset();
	FEditorDelegates::EndPIE.Remove(GameInstanceEndedHandle);
	GameInstanceEndedHandle.Reset();
}

bool FFlickerVoidCoreEditorModule::IsGameInstanceStarted() const
{
	return WeakGameInstance.IsValid();
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

IMPLEMENT_MODULE(FFlickerVoidCoreEditorModule, FlickerVoidCoreEditor)