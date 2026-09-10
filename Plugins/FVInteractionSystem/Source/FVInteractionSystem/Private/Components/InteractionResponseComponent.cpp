#include "Components/InteractionResponseComponent.h"

#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionResponseComponent)

UInteractionResponseComponent::UInteractionResponseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UInteractionResponseComponent* UInteractionResponseComponent::Get(AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UInteractionResponseComponent>() : nullptr;
}

void UInteractionResponseComponent::ReportInteractionProgress(const FInteractionCommit& Commit, float Progress)
{
	OnInteractionProgress.Broadcast(Commit, Progress);
}

void UInteractionResponseComponent::ReportInteractionCancelled(const FInteractionCommit& Commit, const FGameplayTag& Reason)
{
	OnInteractionCancelled.Broadcast(Commit, Reason);
}
