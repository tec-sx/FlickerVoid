#include "Interaction/FVInteractionActionHandler.h"

UFVInteractionActionHandler::UFVInteractionActionHandler()
	: Status(EFVInteractionStatus::Idle)
	, bIsInitialized(false)
{
}

void UFVInteractionActionHandler::Initialize(UFVInteractionHandlerConfig* Config)
{
	if (bIsInitialized)
	{
		return;
	}

	bIsInitialized = true;
	OnInitialize(Config);
}

EFVInteractionResult UFVInteractionActionHandler::Execute(const FFVInteractionContext& Context)
{
	if (Status == EFVInteractionStatus::Executing)
	{
		return EFVInteractionResult::Blocked;
	}

	ActiveContext = Context;
	Status = EFVInteractionStatus::Executing;
	OnExecute(Context);

	return EFVInteractionResult::Success;
}

void UFVInteractionActionHandler::Cancel(const FFVInteractionContext& Context)
{
	if (Status != EFVInteractionStatus::Executing)
	{
		return;
	}

	Status = EFVInteractionStatus::Cancelled;
	OnCancel(Context);
	OnCompleted.Broadcast(Context, EFVInteractionStatus::Cancelled, false);
}

void UFVInteractionActionHandler::CompleteExecution(bool bSuccess)
{
	if (Status != EFVInteractionStatus::Executing)
	{
		return;
	}

	Status = bSuccess ? EFVInteractionStatus::Completed : EFVInteractionStatus::Failed;
	OnCompleted.Broadcast(ActiveContext, Status, bSuccess);

	// Reset to idle so the handler can be reused
	Status = EFVInteractionStatus::Idle;
}
