#include "Components/InteractionSignalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionSignalComponent)

UInteractionSignalComponent::UInteractionSignalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UInteractionSignalComponent* UInteractionSignalComponent::Get(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UInteractionSignalComponent>() : nullptr;
}

