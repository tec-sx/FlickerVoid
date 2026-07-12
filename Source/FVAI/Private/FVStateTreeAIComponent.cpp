#include "FVStateTreeAIComponent.h"

UFVStateTreeAIComponent::UFVStateTreeAIComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFVStateTreeAIComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFVStateTreeAIComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFVStateTreeAIComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UFVStateTreeAIComponent::ValidateStateTreeReference()
{
	Super::ValidateStateTreeReference();
}

void UFVStateTreeAIComponent::StartStateTree(UStateTree* StateTree)
{
	if (!StateTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartStateTree called with null StateTree on %s"), *GetNameSafe(this));
		return;
	}
	
	if (IsRunning())
	{
		Cleanup();
	}
	
	SetStateTree(StateTree);
	StartLogic();
}

void UFVStateTreeAIComponent::StopStateTree()
{
	if (IsRunning())
	{
		Cleanup();
	}
    
	StopLogic(FString("Stopped by Code"));
	SetStateTree(nullptr);
}

void UFVStateTreeAIComponent::SendEvent(const FGameplayTag& EventTag)
{
	if (IsRunning() && LastEventTag != EventTag)
	{
		LastEventTag = EventTag;
		const FStateTreeEvent Event = FStateTreeEvent(EventTag);
		SendStateTreeEvent(Event);
		UE_LOG(
			LogTemp, 
			Warning, 
			TEXT("UFVStateTreeAIComponent::SendEvent - Event '%s' sent."), *EventTag.GetTagName().ToString());
	}
}

