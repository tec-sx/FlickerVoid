#include "Interactions/FVInteractionStateTaskBase.h"
#include "Interactions/FVInteractionTargetComponent.h"

UFVInteractionTargetComponent* UFVInteractionStateTaskBase::GetInteractable(AActor* OwnerActor)
{
	if (!OwnerActor)
	{
		return nullptr;
	}
	return OwnerActor->FindComponentByClass<UFVInteractionTargetComponent>();
}

AActor* UFVInteractionStateTaskBase::GetInstigator(AActor* OwnerActor)
{
	const UFVInteractionTargetComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveInstigator() : nullptr;
}

FGameplayTag UFVInteractionStateTaskBase::GetActionTag(AActor* OwnerActor)
{
	const UFVInteractionTargetComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveActionTag() : FGameplayTag::EmptyTag;
}

FVector UFVInteractionStateTaskBase::GetInteractionPoint(AActor* OwnerActor)
{
	const UFVInteractionTargetComponent* IC = GetInteractable(OwnerActor);
	return IC ? IC->GetActiveInteractionPoint() : FVector::ZeroVector;
}

void UFVInteractionStateTaskBase::CompleteTask(AActor* OwnerActor, bool bSuccess)
{
	if (UFVInteractionTargetComponent* IC = GetInteractable(OwnerActor))
	{
		IC->CompleteActiveTask(bSuccess);
	}
}

EStateTreeRunStatus UFVInteractionStateTaskBase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	return Super::Tick(Context, DeltaTime);
}

// EStateTreeRunStatus UFVInteractionStateTaskBase::ReceiveTick(AActor* OwnerActor, float DeltaTime)
// {

// }
