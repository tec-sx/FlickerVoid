#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"

class UActorComponent;

/** The components of one actor (a class's templates or a placed instance's components) and where rule messages go. */
struct FInteractionCompileContext
{
	using FReport = TFunction<void(EMessageSeverity::Type, const FString&)>;

	FInteractionCompileContext(const TArray<const UActorComponent*>& InComponents, FReport InReport)
		: Components(InComponents)
		, Report(MoveTemp(InReport))
	{}

	void Error(const FString& Message) const { Report(EMessageSeverity::Error, Message); }
	void Warning(const FString& Message) const { Report(EMessageSeverity::Warning, Message); }

	const TArray<const UActorComponent*>& Components;

private:
	FReport Report;
};

class IInteractionCompileRule
{
public:
	virtual ~IInteractionCompileRule() = default;

	virtual FName GetRuleName() const = 0;
	virtual bool ShouldRun(const FInteractionCompileContext& Context) const { return true; }
	virtual void Validate(const FInteractionCompileContext& Context) const = 0;
};
