#include "Components/FVInteractorResponseComponent.h"

#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractableComponent.h"
#include "FVInteractionSystem.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorResponseComponent)

UFVInteractorResponseComponent::UFVInteractorResponseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVInteractorResponseComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwningActor = GetOwner();
	UFVInteractorComponent* Interactor = OwningActor ? OwningActor->FindComponentByClass<UFVInteractorComponent>() : nullptr;

	if (!Interactor)
	{
		UE_LOG(
			LogFVInteraction, 
			Error,
			TEXT("'%s' requires a UFVInteractorComponent on '%s' but none was found."),
			*GetName(), *GetNameSafe(OwningActor));

		return;
	}

	BindEvents(Interactor);
}

void UFVInteractorResponseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const AActor* OwningActor = GetOwner())
	{
		if (UFVInteractorComponent* Interactor = OwningActor->FindComponentByClass<UFVInteractorComponent>())
		{
			UnbindEvents(Interactor);
		}
	}

	Super::EndPlay(EndPlayReason);
}