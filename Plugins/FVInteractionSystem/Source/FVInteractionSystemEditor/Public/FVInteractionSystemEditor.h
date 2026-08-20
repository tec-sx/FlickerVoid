#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UInteractionBlueprintCompilerExtension;

class FFVInteractionSystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterCompilerExtension();
	void UnregisterCompilerExtension();

	TObjectPtr<UInteractionBlueprintCompilerExtension> CompilerExtension;
};
