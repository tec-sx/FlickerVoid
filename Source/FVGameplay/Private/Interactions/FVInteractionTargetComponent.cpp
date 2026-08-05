#include "Interactions/FVInteractionTargetComponent.h"
#include "Interactions/FVInteractionAction.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTree.h"
#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Logging/FVLogCategories.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionTargetComponent)

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
	
	if (StateTreeComponent != nullptr)
	{
		bOwnerHasStateTreeComponent = true;
		StateTreeComponent->SetStartLogicAutomatically(false);
		StateTreeComponent->OnStateTreeRunStatusChanged
			.AddDynamic(this, &UFVInteractionTargetComponent::OnStateTreeStatusChanged);
	}
	
	bIsInitialized = true;
}

void UFVInteractionTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!bIsInitialized)
	{
		return;
	}
	
	CancelActiveInteraction();
	Super::EndPlay(EndPlayReason);
}

bool UFVInteractionTargetComponent::IsInteractionInProgress() const
{
	return StateTreeComponent && StateTreeComponent->IsRunning();
}

void UFVInteractionTargetComponent::RunAction(AActor* Instigator, UFVInteractionAction* Action)
{
	ActiveInstigator      = Instigator;
	ActiveActionTag       = Action->ActionTag;
	ActiveInteractionPoint = GetOwner()->GetActorLocation();
	bActiveTaskDone       = false;
	bActiveTaskSucceeded  = false;
	CompletingActionTag   = Action->ActionTag;
	
#if UE_BUILD_DEVELOPMENT
	if (!bOwnerHasStateTreeComponent)
	{
		UE_LOG(
			LogFVGameplay, 
			Warning, 
			TEXT("INTERACTION: Complex action was executed, but %s does not have StateTreeComponent"), *GetOwner()->GetName());
	}
#endif
	
	if (bOwnerHasStateTreeComponent && !Action->bIsSimple)
	{
		StateTreeComponent->SetStateTree(Action->ActionStateTree);
		StateTreeComponent->StartLogic();
	}
}

void UFVInteractionTargetComponent::CancelActiveInteraction()
{
	if (!bIsInitialized || !bOwnerHasStateTreeComponent)
	{
		return;
	}
	
	if (StateTreeComponent && StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(TEXT("Cancelled"));
	}
}

void UFVInteractionTargetComponent::CompleteActiveTask(bool bSuccess)
{
	if (!bIsInitialized)
	{
		return;
	}
	
	bActiveTaskDone      = true;
	bActiveTaskSucceeded = bSuccess;
}

void UFVInteractionTargetComponent::SetFocused(bool bFocused)
{
	if (!bIsInitialized)
	{
		return;
	}
	
	if (bFocused == bIsInFocus)
	{
		return;
	}

	bIsInFocus = bFocused;
}

//~=============================================================================
// State Tree completion
//~=============================================================================

void UFVInteractionTargetComponent::OnStateTreeStatusChanged(EStateTreeRunStatus RunStatus)
{
	if (!bIsInitialized)
	{
		return;
	}
	
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
	
	CancelActiveInteraction();
	OnAnyActionCompleted.Broadcast(Context, Status, bSuccess);
}
