#include "Interaction/FVInteractionStateTreeTaskBase.h"
#include "Interaction/FVInteractableComponent.h"

UFVInteractableComponent* UFVInteractionStateTreeTaskBase::GetInteractable(AActor* OwnerActor)
{
	if (!OwnerActor)
	{
		return nullptr;
	}
	return OwnerActor->FindComponentByClass<UFVInteractableComponent>();
}

AActor* UFVInteractionStateTreeTaskBase::GetInstigator(AActor* OwnerActor)
{
	const UFVInteractableComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveInstigator() : nullptr;
}

FGameplayTag UFVInteractionStateTreeTaskBase::GetActionTag(AActor* OwnerActor)
{
	const UFVInteractableComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveActionTag() : FGameplayTag::EmptyTag;
}

FVector UFVInteractionStateTreeTaskBase::GetInteractionPoint(AActor* OwnerActor)
{
	const UFVInteractableComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveInteractionPoint() : FVector::ZeroVector;
}

void UFVInteractionStateTreeTaskBase::CompleteTask(AActor* OwnerActor, bool bSuccess)
{
	if (UFVInteractableComponent* IC = GetInteractable(OwnerActor))
	{
		IC->CompleteActiveTask(bSuccess);
	}
}

EStateTreeRunStatus UFVInteractionStateTreeTaskBase::ReceiveTick_Implementation(
	AActor* OwnerActor, float DeltaTime)
{
	if (const UFVInteractableComponent* IC = GetInteractable(OwnerActor))
	{
		if (IC->IsActiveTaskDone())
		{
			return IC->DidActiveTaskSucceed()
				? EStateTreeRunStatus::Succeeded
				: EStateTreeRunStatus::Failed;
		}
	}
	return EStateTreeRunStatus::Running;
}
