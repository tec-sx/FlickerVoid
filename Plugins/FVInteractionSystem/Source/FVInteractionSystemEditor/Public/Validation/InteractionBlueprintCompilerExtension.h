#pragma once

#include "CoreMinimal.h"
#include "BlueprintCompilerExtension.h"
#include "InteractionBlueprintCompilerExtension.generated.h"

UCLASS()
class UInteractionBlueprintCompilerExtension : public UBlueprintCompilerExtension
{
	GENERATED_BODY()

protected:
	virtual void ProcessBlueprintCompiled(const FKismetCompilerContext& CompilationContext, const FBlueprintCompiledData& Data) override;
};
