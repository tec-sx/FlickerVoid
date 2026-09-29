#include "Validation/InteractionBlueprintCompilerExtension.h"

#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "KismetCompiler.h"
#include "Validation/InteractionCompileRule.h"
#include "Validation/InteractionCompileRuleRegistry.h"

void UInteractionBlueprintCompilerExtension::ProcessBlueprintCompiled(const FKismetCompilerContext& CompilationContext, const FBlueprintCompiledData& Data)
{
	const UBlueprint* Blueprint = CompilationContext.Blueprint;
	if (Blueprint)
	{
		const FInteractionCompileContext Context(*Blueprint, CompilationContext.MessageLog);

		for (const TSharedRef<IInteractionCompileRule>& Rule : FInteractionCompileRuleRegistry::Get().GetRules())
		{
			if (Rule->ShouldRun(Context))
			{
				Rule->Validate(Context);
			}
		}
	}
}
