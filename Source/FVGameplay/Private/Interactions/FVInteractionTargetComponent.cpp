#include "Interactions/FVInteractionTargetComponent.h"
#include "Interactions/FVInteractionAction.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTree.h"

UFVInteractionTargetComponent::UFVInteractionTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVInteractionTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return;
	}
	
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction Instigator Configuration not found. creating default object."));
		Config = NewObject<UFVInteractionTargetConfig>();
	}
	
	StateTreeComponent = Owner->GetComponentByClass<UStateTreeComponent>();
	
	if (IsValid(StateTreeComponent))
	{
		StateTreeComponent->SetStartLogicAutomatically(false);
		StateTreeComponent->OnStateTreeRunStatusChanged
			.AddDynamic(this, &UFVInteractionTargetComponent::OnStateTreeStatusChanged);
	}
	else
	{
		bIsSimple = true;
	}
}

void UFVInteractionTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelActiveInteraction();
	Super::EndPlay(EndPlayReason);
}

//~=============================================================================
// UI Data
//~=============================================================================

bool UFVInteractionTargetComponent::IsInteractionInProgress() const
{
	return StateTreeComponent && StateTreeComponent->IsRunning();
}

//~=============================================================================
// Execution
//~=============================================================================

EFVInteractionResult UFVInteractionTargetComponent::TryExecuteAction(
	const FGameplayTag& InputTag, 
	AActor* Instigator,
	FGameplayTagContainer& InstigatorTags)
{
	if (IsInteractionInProgress())
	{
		return EFVInteractionResult::Blocked;
	}

	for (const UFVInteractionAction* Action : Config->AvailableActions)
	{
		if (!Action->InputTag.MatchesTagExact(InputTag))
		{
			continue;
		}
		
		if (!Action->ActionStateTree)
		{
			continue;
		}

		FText UnmetReason;
		if (!Action->CheckRequirements(InstigatorTags))
		{
			return EFVInteractionResult::RequirementNotMet;
		}

		// Store context so tasks can read it via GetActive* accessors
		ActiveInstigator      = Instigator;
		ActiveActionTag       = Action->ActionTag;
		ActiveInteractionPoint = GetOwner()->GetActorLocation();
		bActiveTaskDone       = false;
		bActiveTaskSucceeded  = false;
		CompletingActionTag   = Action->ActionTag;

		StateTreeComponent->SetStateTree(Action->ActionStateTree);
		StateTreeComponent->StartLogic();

		return EFVInteractionResult::Success;
	}

	return EFVInteractionResult::ActionNotFound;
}

void UFVInteractionTargetComponent::CancelActiveInteraction()
{
	if (StateTreeComponent && StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(TEXT("Cancelled"));
	}
}

void UFVInteractionTargetComponent::CompleteActiveTask(bool bSuccess)
{
	bActiveTaskDone      = true;
	bActiveTaskSucceeded = bSuccess;
}

void UFVInteractionTargetComponent::SetFocused(bool bFocused)
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

void UFVInteractionTargetComponent::OnStateTreeStatusChanged(EStateTreeRunStatus RunStatus)
{
	const bool bSuccess = (RunStatus == EStateTreeRunStatus::Succeeded);

	if (bSuccess)
	{
		for (UFVInteractionAction* Action : GetAvailableActions())
		{
			if (Action->ActionTag.MatchesTagExact(CompletingActionTag))
			{
				if (!Action->GetGrantedTags().IsEmpty())
				{
					if (UAbilitySystemComponent* ASC =
						UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActiveInstigator.Get()))
					{
						ASC->AddLooseGameplayTags(Action->GetGrantedTags());
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
