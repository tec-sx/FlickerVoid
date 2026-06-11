#include "Interaction/FVInteractionStateTaskBase.h"
#include "Interaction/FVInteractableComponent.h"

UFVInteractableComponent* UFVInteractionStateTaskBase::GetInteractable(AActor* OwnerActor)
{
	if (!OwnerActor)
	{
		return nullptr;
	}
	return OwnerActor->FindComponentByClass<UFVInteractableComponent>();
}

AActor* UFVInteractionStateTaskBase::GetInstigator(AActor* OwnerActor)
{
	const UFVInteractableComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveInstigator() : nullptr;
}

FGameplayTag UFVInteractionStateTaskBase::GetActionTag(AActor* OwnerActor)
{
	const UFVInteractableComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveActionTag() : FGameplayTag::EmptyTag;
}

FVector UFVInteractionStateTaskBase::GetInteractionPoint(AActor* OwnerActor)
{
	const UFVInteractableComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveInteractionPoint() : FVector::ZeroVector;
}

void UFVInteractionStateTaskBase::CompleteTask(AActor* OwnerActor, bool bSuccess)
{
	if (UFVInteractableComponent* IC = GetInteractable(OwnerActor))
	{
		IC->CompleteActiveTask(bSuccess);
	}
}

// EStateTreeRunStatus UFVInteractionStateTaskBase::ReceiveTick(AActor* OwnerActor, float DeltaTime)
// {
// 	if (const UFVInteractableComponent* IC = GetInteractable(OwnerActor))
// 	{
// 		if (IC->IsActiveTaskDone())
// 		{
// 			return IC->DidActiveTaskSucceed() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
// 		}
// 	}
// 	
// 	return EStateTreeRunStatus::Running;
// }
