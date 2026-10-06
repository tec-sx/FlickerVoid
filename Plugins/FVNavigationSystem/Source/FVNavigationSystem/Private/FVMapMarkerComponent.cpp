#include "FVMapMarkerComponent.h"

#include "FVMarkerDefinition.h"
#include "FVNavigationSubsystem.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMapMarkerComponent)

UFVMapMarkerComponent::UFVMapMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVMapMarkerComponent* UFVMapMarkerComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVMapMarkerComponent>() : nullptr;
}

void UFVMapMarkerComponent::SetDefinition(UFVMarkerDefinition* NewDefinition)
{
	if (Definition == NewDefinition)
	{
		return;
	}

	Unregister();
	Definition = NewDefinition;
	Register();
}

void UFVMapMarkerComponent::SetMarkerEnabled(bool bEnabled)
{
	if (bMarkerEnabled == bEnabled)
	{
		return;
	}

	bMarkerEnabled = bEnabled;
	if (bMarkerEnabled)
	{
		Register();
	}
	else
	{
		Unregister();
	}
}

FText UFVMapMarkerComponent::GetLabel() const
{
	if (!Label.IsEmpty() || Definition == nullptr)
	{
		return Label;
	}
	return Definition->Display.Name;
}

bool UFVMapMarkerComponent::IsDiscovered() const
{
	const UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	return Navigation && Navigation->IsMarkerDiscovered(Handle);
}

bool UFVMapMarkerComponent::Discover(AActor* Discoverer)
{
	UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	return Navigation && Navigation->DiscoverMarker(Handle, Discoverer);
}

void UFVMapMarkerComponent::BeginPlay()
{
	Super::BeginPlay();
	Register();
}

void UFVMapMarkerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Unregister();
	Super::EndPlay(EndPlayReason);
}

void UFVMapMarkerComponent::Register()
{
	if (Handle.IsValid() || !bMarkerEnabled || Definition == nullptr || !HasBegunPlay())
	{
		return;
	}

	if (UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this))
	{
		Handle = Navigation->RegisterComponent(this);
	}
}

void UFVMapMarkerComponent::Unregister()
{
	if (!Handle.IsValid())
	{
		return;
	}

	if (UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this))
	{
		Navigation->RemoveMarker(Handle);
	}
	Handle.Reset();
}
