#include "Validation/InteractionBlueprintCompilerExtension.h"

#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "Kismet2/CompilerResultsLog.h"
#include "KismetCompiler.h"
#include "Validation/InteractionBlueprintComponentUtils.h"
#include "Validation/InteractionCompileRule.h"

void UInteractionBlueprintCompilerExtension::ProcessBlueprintCompiled(const FKismetCompilerContext& CompilationContext, const FBlueprintCompiledData& Data)
{
	const UBlueprint* Blueprint = CompilationContext.Blueprint;
	const UClass* ActorClass = Blueprint ? Blueprint->GeneratedClass.Get() : nullptr;
	if (!ActorClass || !ActorClass->IsChildOf<AActor>())
	{
		return;
	}

	TArray<const UActorComponent*> Templates;
	InteractionBlueprintComponentUtils::CollectComponentTemplates(ActorClass, Templates);

	FCompilerResultsLog& MessageLog = CompilationContext.MessageLog;
	const FInteractionCompileContext Context(Templates, [&MessageLog](const EMessageSeverity::Type Severity, const FString& Message)
	{
		if (Severity == EMessageSeverity::Error)
		{
			MessageLog.Error(*Message);
			return;
		}
		MessageLog.Warning(*Message);
	});
	InteractionBlueprintComponentUtils::RunRules(Context);
}
