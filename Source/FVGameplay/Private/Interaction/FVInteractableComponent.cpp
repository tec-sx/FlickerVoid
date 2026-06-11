#include "Interaction/FVInteractableComponent.h"
#include "Interaction/FVInteractionAction.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTree.h"

UFVInteractableComponent::UFVInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	CreateStateTreeComponent();
}

void UFVInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelActiveInteraction();
	Super::EndPlay(EndPlayReason);
}

//~=============================================================================
// Initialization
//~=============================================================================

void UFVInteractableComponent::CreateStateTreeComponent()
{
	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return;
	}

	InteractionStateTreeComp = NewObject<UStateTreeComponent>(Owner, UStateTreeComponent::StaticClass(), TEXT("InteractionStateTree"));
	InteractionStateTreeComp->SetStartLogicAutomatically(false);
	Owner->AddOwnedComponent(InteractionStateTreeComp);
	InteractionStateTreeComp->RegisterComponent();

	InteractionStateTreeComp->OnStateTreeRunStatusChanged.AddDynamic(this, &UFVInteractableComponent::OnStateTreeStatusChanged);
}

//~=============================================================================
// UI Data
//~=============================================================================

bool UFVInteractableComponent::IsBeingInteracted() const
{
	return InteractionStateTreeComp && InteractionStateTreeComp->IsRunning();
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
		Entry.bAvailable        = Action.CheckRequirements(Instigator, UnmetReason);
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

	for (const FFVInteractionAction& Action : Actions)
	{
		if (!Action.InputTag.MatchesTagExact(InputTag))
		{
			continue;
		}

		if (!Action.ActionStateTree)
		{
			continue;
		}

		FText UnmetReason;
		if (!Action.CheckRequirements(Instigator, UnmetReason))
		{
			return EFVInteractionResult::RequirementNotMet;
		}

		// Store context so tasks can read it via GetActive* accessors
		ActiveInstigator      = Instigator;
		ActiveActionTag       = Action.ActionTag;
		ActiveInteractionPoint = GetOwner()->GetActorLocation();
		bActiveTaskDone       = false;
		bActiveTaskSucceeded  = false;
		CompletingActionTag   = Action.ActionTag;

		InteractionStateTreeComp->SetStateTree(Action.ActionStateTree);
		InteractionStateTreeComp->StartLogic();

		return EFVInteractionResult::Success;
	}

	return EFVInteractionResult::ActionNotFound;
}

void UFVInteractableComponent::CancelActiveInteraction()
{
	if (InteractionStateTreeComp && InteractionStateTreeComp->IsRunning())
	{
		InteractionStateTreeComp->StopLogic(TEXT("Cancelled"));
	}
}

void UFVInteractableComponent::CompleteActiveTask(bool bSuccess)
{
	bActiveTaskDone      = true;
	bActiveTaskSucceeded = bSuccess;
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
// State Tree completion
//~=============================================================================

void UFVInteractableComponent::OnStateTreeStatusChanged(EStateTreeRunStatus RunStatus)
{
	const bool bSuccess = (RunStatus == EStateTreeRunStatus::Succeeded);

	if (bSuccess)
	{
		for (const FFVInteractionAction& Action : Actions)
		{
			if (Action.ActionTag.MatchesTagExact(CompletingActionTag))
			{
				if (!Action.GrantedTagsOnSuccess.IsEmpty())
				{
					if (UAbilitySystemComponent* ASC =
						UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActiveInstigator.Get()))
					{
						ASC->AddLooseGameplayTags(Action.GrantedTagsOnSuccess);
					}
				}
				break;
			}
		}
	}

	FFVInteractionContext Context;
	Context.Instigator       = ActiveInstigator.Get();
	Context.TargetActor      = GetOwner();
	Context.TargetComponent  = this;
	Context.ActionTag        = CompletingActionTag;
	Context.InteractionPoint = ActiveInteractionPoint;

	EFVInteractionStatus Status;
	switch (RunStatus)
	{
		case EStateTreeRunStatus::Succeeded: Status = EFVInteractionStatus::Completed;  break;
		case EStateTreeRunStatus::Failed:    Status = EFVInteractionStatus::Failed;     break;
		default:                             Status = EFVInteractionStatus::Cancelled;  break;
	}

	// Clear active context before broadcast so re-entrant calls see a clean state
	ActiveInstigator    = nullptr;
	ActiveActionTag     = FGameplayTag::EmptyTag;
	bActiveTaskDone     = false;
	bActiveTaskSucceeded = false;

	OnAnyActionCompleted.Broadcast(Context, Status, bSuccess);
}
