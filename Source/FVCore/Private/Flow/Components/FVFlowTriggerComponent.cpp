#include "Flow/Components/FVFlowTriggerComponent.h"

#include "FVCoreTags.h"
#include "Components/ShapeComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowTriggerComponent)

UFVFlowTriggerComponent::UFVFlowTriggerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bAutoEnable(true)
	, bOverlapEnabled(false)
{}

void UFVFlowTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (bAutoEnable)
	{
		EnableOverlap();
	}
}

void UFVFlowTriggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisableOverlap();
	
	Super::EndPlay(EndPlayReason);
}

void UFVFlowTriggerComponent::EnableOverlap()
{
	if (!bOverlapEnabled)
	{
		const FName Tag = FVCoreTags::Flow_Common_TriggerZone.GetTag().GetTagName();
		if (UShapeComponent* TriggerZone = GetOwner()->FindComponentByTag<UShapeComponent>(Tag))
		{
			TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &UFVFlowTriggerComponent::OnComponentBeginOverlap);
			TriggerZone->OnComponentEndOverlap.AddDynamic(this, &UFVFlowTriggerComponent::OnComponentEndOverlap);
			TriggerZonePtr = TriggerZone;
		}
	}
}

void UFVFlowTriggerComponent::DisableOverlap()
{
	if (bOverlapEnabled)
	{
		bOverlapEnabled = false;
		
		if (UShapeComponent* TriggerZone = TriggerZonePtr.Get())
		{
			TriggerZone->OnComponentBeginOverlap.RemoveAll(this);
			TriggerZone->OnComponentEndOverlap.RemoveAll(this);
		}
	}
}

void UFVFlowTriggerComponent::NotifyFromGraph(const FGameplayTagContainer& NotifyTags, const EFlowNetMode NetMode)
{
	if (NotifyTags.HasTagExact(FVCoreTags::Flow_Common_Enable))
	{
		EnableOverlap();
	}
	else if (NotifyTags.HasTagExact(FVCoreTags::Flow_Common_Disable))
	{
		DisableOverlap();
	}
	
	Super::NotifyFromGraph(NotifyTags, NetMode);
}

void UFVFlowTriggerComponent::OnComponentBeginOverlap(
	UPrimitiveComponent* InOverlappedComponent, 
	AActor* InOtherActor,
	UPrimitiveComponent* InOtherComp, 
	int32 InOtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& InSweepResult)
{
	if (InOtherActor != GetOwner())
	{
		if (UFlowComponent* FlowComponent = InOtherActor->FindComponentByClass<UFlowComponent>())
		{
			FlowComponentPtr = FlowComponent;
			if (GetOwner()->HasActorBegunPlay())
			{
				OnTriggerEvent.Broadcast(true, FlowComponent);
			}
			else
			{
				TWeakObjectPtr<UFVFlowTriggerComponent> SelfWeakPtr(this);
				TWeakObjectPtr<UFlowComponent> FlowComponentWeakPtr(FlowComponent);
				
				GetWorld()->GetTimerManager().SetTimerForNextTick([SelfWeakPtr, FlowComponentWeakPtr]()
				{
					SelfWeakPtr->OnTriggerEvent.Broadcast(true, FlowComponentWeakPtr.Get());
				});
			}
		}
	}
}

void UFVFlowTriggerComponent::OnComponentEndOverlap(
	UPrimitiveComponent* InOverlappedComponent, 
	AActor* InOtherActor,
	UPrimitiveComponent* InOtherComp, 
	int32 InOtherBodyIndex)
{
	if (UFlowComponent* FlowComponent = FlowComponentPtr.Get())
	{
		OnTriggerEvent.Broadcast(false, FlowComponent);
	}
}
