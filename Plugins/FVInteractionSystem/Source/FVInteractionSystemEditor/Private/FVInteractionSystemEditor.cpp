#include "FVInteractionSystemEditor.h"

#include "BlueprintCompilationManager.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"
#include "Validation/InteractionBlueprintCompilerExtension.h"
#include "Validation/InteractionCompileRuleRegistry.h"

#define LOCTEXT_NAMESPACE "FFVInteractionSystemEditorModule"

void FFVInteractionSystemEditorModule::StartupModule()
{
	FInteractionCompileRuleRegistry::Get().RegisterDefaultRules();
	RegisterCompilerExtension();
}

void FFVInteractionSystemEditorModule::ShutdownModule()
{
	UnregisterCompilerExtension();
	FInteractionCompileRuleRegistry::Get().Reset();
}

void FFVInteractionSystemEditorModule::RegisterCompilerExtension()
{
	CompilerExtension = NewObject<UInteractionBlueprintCompilerExtension>(GetTransientPackage());
	CompilerExtension->AddToRoot();

	FBlueprintCompilationManager::RegisterCompilerExtension(UBlueprint::StaticClass(), CompilerExtension);
}

void FFVInteractionSystemEditorModule::UnregisterCompilerExtension()
{
	if (CompilerExtension && !GExitPurge)
	{
		CompilerExtension->RemoveFromRoot();
	}

	CompilerExtension = nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVInteractionSystemEditorModule, FVInteractionSystemEditor)
