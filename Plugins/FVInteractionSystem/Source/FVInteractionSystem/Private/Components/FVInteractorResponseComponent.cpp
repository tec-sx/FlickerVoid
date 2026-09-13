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

	const AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	if (UFVInteractorComponent* Interactor = GetOwner()->GetComponentByClass<UFVInteractorComponent>())
	{
		BindEvents(Interactor);
	}
	else
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' on '%s' can not find an signal component to bind to."),
			*GetName(), 
			*OwningActor->GetName());
	}
}

void UFVInteractorResponseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVInteractorComponent* Interactor = GetOwner()->GetComponentByClass<UFVInteractorComponent>())
	{
		UnbindEvents(Interactor);	
	}
	
	Super::EndPlay(EndPlayReason);
}