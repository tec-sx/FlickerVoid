#include "Interactions/UFVInteractionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UFVInteractionComponent)

FPlayerInInteractionEvent UInteractionComponent::OnPlayerEnter;
FPlayerInInteractionEvent UInteractionComponent::OnPlayerExit;

// Sets default values for this component's properties
UUFVInteractionComponent::UUFVInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Distance(100.0f)
{
	bAutoActivate = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	SetUsingAbsoluteScale(true);
	ArrowColor
}


// Called when the game starts
void UUFVInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUFVInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUFVInteractionComponent::Enable()
{
}

void UUFVInteractionComponent::Disable()
{
}

