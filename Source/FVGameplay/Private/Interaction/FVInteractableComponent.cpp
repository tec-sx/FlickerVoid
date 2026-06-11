#include "Interaction/FVInteractableComponent.h"
#include "Interaction/FVInteractionActionHandler.h"
#include "Interaction/FVInteractionRequirement.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UFVInteractableComponent::UFVInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeHandlers();
}

void UFVInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelActiveInteraction();
	Super::EndPlay(EndPlayReason);
}

//~=============================================================================
// Initialization
//~=============================================================================

void UFVInteractableComponent::InitializeHandlers()
{
	HandlerInstances.Empty();

	for (const FFVInteractionAction& Action : Actions)
	{
		if (!Action.HandlerClass)
		{
			HandlerInstances.Add(nullptr);
			continue;
		}

		UFVInteractionActionHandler* Handler = NewObject<UFVInteractionActionHandler>(this, Action.HandlerClass);
		if (Handler)
		{
			Handler->Initialize(Action.HandlerConfig);
			Handler->OnCompleted.AddDynamic(this, &UFVInteractableComponent::HandleActionCompleted);
		}

		HandlerInstances.Add(Handler);
	}
}

//~=============================================================================
// UI Data
//~=============================================================================

bool UFVInteractableComponent::IsBeingInteracted() const
{
	return ActiveHandler != nullptr && ActiveHandler->IsExecuting();
}

TArray<FFVInteractionActionDisplay> UFVInteractableComponent::GetActionDisplayData(AActor* Instigator) const
{
	TArray<FFVInteractionActionDisplay> DisplayData;
	DisplayData.Reserve(Actions.Num());

	for (const FFVInteractionAction& Action : Actions)
	{
		FFVInteractionActionDisplay Entry;
		Entry.ActionTag   = Action.ActionTag;
		Entry.InputTag    = Action.InputTag;
		Entry.DisplayName = Action.DisplayName;
		Entry.Icon        = Action.Icon;

		FText UnmetReason;
		Entry.bAvailable      = Action.CheckRequirements(Instigator, UnmetReason);
		Entry.UnavailableReason = UnmetReason;

		DisplayData.Add(Entry);
	}

	return DisplayData;
}

//~=============================================================================
// Execution
//~=============================================================================

EFVInteractionResult UFVInteractableComponent::TryExecuteAction(const FGameplayTag& InputTag, AActor* Instigator)
{
	if (IsBeingInteracted())
	{
		return EFVInteractionResult::Blocked;
	}

	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		const FFVInteractionAction& Action = Actions[i];
		if (!Action.InputTag.MatchesTagExact(InputTag))
		{
			continue;
		}

		UFVInteractionActionHandler* Handler = HandlerInstances.IsValidIndex(i) ? HandlerInstances[i] : nullptr;
		if (!Handler)
		{
			continue;
		}

		FText UnmetReason;
		if (!Action.CheckRequirements(Instigator, UnmetReason))
		{
			return EFVInteractionResult::RequirementNotMet;
		}

		FFVInteractionContext Context;
		Context.Instigator       = Instigator;
		Context.TargetActor      = GetOwner();
		Context.TargetComponent  = this;
		Context.ActionTag        = Action.ActionTag;
		Context.InteractionPoint = GetOwner()->GetActorLocation();

		EFVInteractionResult Result = Handler->Execute(Context);

		if (Result == EFVInteractionResult::Success)
		{
			ActiveHandler = Handler;
		}

		return Result;
	}

	return EFVInteractionResult::ActionNotFound;
}

void UFVInteractableComponent::CancelActiveInteraction()
{
	if (ActiveHandler && ActiveHandler->IsExecuting())
	{
		const FFVInteractionContext& Ctx = ActiveHandler->GetActiveContext();
		ActiveHandler->Cancel(Ctx);
	}

	ActiveHandler = nullptr;
}

void UFVInteractableComponent::SetFocused(bool bFocused)
{
	if (bFocused == bIsInFocus)
	{
		return;
	}

	bIsInFocus = bFocused;
	OnFocusChanged.Broadcast(bFocused);
}

//~=============================================================================
// Completion callback
//~=============================================================================

void UFVInteractableComponent::HandleActionCompleted(const FFVInteractionContext& Context,
	EFVInteractionStatus Status, bool bSuccess)
{
	// Find the action and apply granted tags on success
	if (bSuccess)
	{
		for (const FFVInteractionAction& Action : Actions)
		{
			if (Action.ActionTag.MatchesTagExact(Context.ActionTag))
			{
				if (!Action.GrantedTagsOnSuccess.IsEmpty())
				{
					if (UAbilitySystemComponent* ASC =
						UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Context.Instigator))
					{
						ASC->AddLooseGameplayTags(Action.GrantedTagsOnSuccess);
					}
				}
				break;
			}
		}
	}

	ActiveHandler = nullptr;
	OnAnyActionCompleted.Broadcast(Context, Status, bSuccess);
}
