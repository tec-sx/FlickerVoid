#pragma once

#include "CoreMinimal.h"

class UBlueprint;
class FCompilerResultsLog;

struct FInteractionCompileContext
{
	FInteractionCompileContext(const UBlueprint& InBlueprint, FCompilerResultsLog& InMessageLog)
		: Blueprint(InBlueprint)
		, MessageLog(InMessageLog)
	{}

	const UBlueprint& Blueprint;
	FCompilerResultsLog& MessageLog;
};


class IInteractionCompileRule
{
public:
	virtual ~IInteractionCompileRule() = default;

	virtual FName GetRuleName() const = 0;
	virtual bool ShouldRun(const FInteractionCompileContext& Context) const { return true; }
	virtual void Validate(const FInteractionCompileContext& Context) const = 0;
};
