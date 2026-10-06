#include "FVNavigationSubsystem.h"

#include "Conditions/FVCondition.h"
#include "Engine/World.h"
#include "Facts/FVFactDatabase.h"
#include "FVMapDefinition.h"
#include "FVMapMarkerComponent.h"
#include "FVMarkerDefinition.h"
#include "FVNavigationSystem.h"
#include "FVNavigatorComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNavigationSubsystem)

FVector FFVMarker::GetLocation() const
{
	const UFVMapMarkerComponent* Marker = Component.Get();
	return Marker ? Marker->GetComponentLocation() : Location;
}

float FFVMarker::GetYaw() const
{
	const AActor* Actor = GetActor();
	return Actor ? Actor->GetActorRotation().Yaw : 0.f;
}

AActor* FFVMarker::GetActor() const
{
	const UFVMapMarkerComponent* Marker = Component.Get();
	return Marker ? Marker->GetOwner() : nullptr;
}

FText FFVMarker::GetLabel() const
{
	if (const UFVMapMarkerComponent* Marker = Component.Get())
	{
		return Marker->GetLabel();
	}
	return Label.IsEmpty() && Definition ? Definition->Display.Name : Label;
}

UFVNavigationSubsystem* UFVNavigationSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext != nullptr ? WorldContext->GetWorld() : nullptr;
	return World != nullptr ? World->GetSubsystem<UFVNavigationSubsystem>() : nullptr;
}

bool UFVNavigationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVNavigationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	for (const TSoftObjectPtr<UFVMapDefinition>& Soft : UFVNavigationSettings::Get().Maps)
	{
		if (UFVMapDefinition* Map = Soft.LoadSynchronous())
		{
			Maps.Add(Map);
		}
	}

	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		FactChangedHandle = Facts->OnFactChangedNative().AddUObject(this, &UFVNavigationSubsystem::HandleFactChanged);
	}
}

void UFVNavigationSubsystem::Deinitialize()
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->OnFactChangedNative().Remove(FactChangedHandle);
	}
	FactChangedHandle.Reset();
	Markers.Reset();
	Maps.Reset();

	Super::Deinitialize();
}

UFVMapDefinition* UFVNavigationSubsystem::FindMapAt(const FVector& Location, int32& OutLayerIndex) const
{
	FFVConditionContext Context;
	Context.WorldContext = const_cast<UFVNavigationSubsystem*>(this);
	Context.Instigator = Navigator.IsValid() ? Navigator->GetOwner() : nullptr;

	UFVMapDefinition* Best = nullptr;
	OutLayerIndex = INDEX_NONE;

	for (UFVMapDefinition* Map : Maps)
	{
		if (Map == nullptr || (Best != nullptr && Map->Priority <= Best->Priority))
		{
			continue;
		}

		const int32 LayerIndex = Map->FindLayerAt(Location);
		if (LayerIndex != INDEX_NONE && Map->AvailableWhen.Evaluate(Context))
		{
			Best = Map;
			OutLayerIndex = LayerIndex;
		}
	}
	return Best;
}

TArray<UFVMapDefinition*> UFVNavigationSubsystem::GetMaps() const
{
	TArray<UFVMapDefinition*> Result;
	Result.Reserve(Maps.Num());
	for (UFVMapDefinition* Map : Maps)
	{
		Result.Add(Map);
	}
	return Result;
}

FFVMarkerHandle UFVNavigationSubsystem::RegisterComponent(UFVMapMarkerComponent* Component)
{
	if (Component == nullptr || Component->GetDefinition() == nullptr)
	{
		return FFVMarkerHandle();
	}

	FFVMarker Marker;
	Marker.Definition = Component->GetDefinition();
	Marker.Component = Component;
	return AddMarker(MoveTemp(Marker));
}

FFVMarkerHandle UFVNavigationSubsystem::AddMarkerAtLocation(UFVMarkerDefinition* Definition, FVector Location, FText Label)
{
	if (Definition == nullptr)
	{
		return FFVMarkerHandle();
	}

	FFVMarker Marker;
	Marker.Definition = Definition;
	Marker.Location = Location;
	Marker.Label = Label;
	return AddMarker(MoveTemp(Marker));
}

FFVMarkerHandle UFVNavigationSubsystem::AddMarker(FFVMarker&& Marker)
{
	Marker.Handle.Id = NextMarkerId++;
	Marker.bDiscovered = Marker.Definition->FindFragment<FFVMarkerFragment_Discovery>() == nullptr;
	UpdateMarker(Marker);

	const FFVMarkerHandle Handle = Marker.Handle;
	Markers.Add(Handle.Id, MoveTemp(Marker));
	OnMarkerAdded.Broadcast(Handle);
	return Handle;
}

void UFVNavigationSubsystem::RemoveMarker(FFVMarkerHandle Marker)
{
	if (Markers.Remove(Marker.Id) == 0)
	{
		return;
	}

	if (Waypoint == Marker)
	{
		Waypoint.Reset();
		OnWaypointChanged.Broadcast(Waypoint);
	}

	if (TrackedMarker == Marker)
	{
		SetTrackedMarker(FFVMarkerHandle());
	}

	OnMarkerRemoved.Broadcast(Marker);
}

bool UFVNavigationSubsystem::IsMarkerVisible(FFVMarkerHandle Marker) const
{
	const FFVMarker* Found = FindMarker(Marker);
	return Found && Found->bVisible;
}

bool UFVNavigationSubsystem::IsMarkerDiscovered(FFVMarkerHandle Marker) const
{
	const FFVMarker* Found = FindMarker(Marker);
	return Found && Found->bDiscovered;
}

FVector UFVNavigationSubsystem::GetMarkerLocation(FFVMarkerHandle Marker) const
{
	const FFVMarker* Found = FindMarker(Marker);
	return Found ? Found->GetLocation() : FVector::ZeroVector;
}

UFVMarkerDefinition* UFVNavigationSubsystem::GetMarkerDefinition(FFVMarkerHandle Marker) const
{
	const FFVMarker* Found = FindMarker(Marker);
	return Found ? Found->Definition.Get() : nullptr;
}

AActor* UFVNavigationSubsystem::GetMarkerActor(FFVMarkerHandle Marker) const
{
	const FFVMarker* Found = FindMarker(Marker);
	return Found ? Found->GetActor() : nullptr;
}

TArray<FFVMarkerHandle> UFVNavigationSubsystem::GetMarkerHandles() const
{
	TArray<FFVMarkerHandle> Handles;
	Handles.Reserve(Markers.Num());
	for (const TPair<int32, FFVMarker>& Pair : Markers)
	{
		Handles.Add(Pair.Value.Handle);
	}
	return Handles;
}

bool UFVNavigationSubsystem::DiscoverMarker(FFVMarkerHandle Marker, AActor* Discoverer)
{
	FFVMarker* Found = Markers.Find(Marker.Id);
	if (Found == nullptr || Found->bDiscovered)
	{
		return false;
	}

	const FFVMarkerFragment_Discovery* Discovery = Found->Definition->FindFragment<FFVMarkerFragment_Discovery>();
	if (Discovery == nullptr)
	{
		return false;
	}

	Found->bDiscovered = true;

	// Writing the fact and applying effects can add or remove markers, so nothing below reads Found.
	UFVMapMarkerComponent* Component = Found->Component.Get();
	const FFVConditionContext Context = MakeContext(*Found, Discoverer ? Discoverer : (Navigator.IsValid() ? Navigator->GetOwner() : nullptr));
	const FFVEffectList& Effects = Discovery->OnDiscovered;

	if (Component != nullptr && Component->GetDiscoveredFact().IsValid())
	{
		if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
		{
			Facts->SetFact(Component->GetDiscoveredFact(), 1);
		}
	}

	Effects.Apply(Context);

	OnMarkerDiscovered.Broadcast(Marker);
	if (Component != nullptr)
	{
		Component->OnDiscovered.Broadcast(Component);
	}
	return true;
}

void UFVNavigationSubsystem::RefreshMarkers()
{
	TArray<FFVMarkerHandle> Changed;
	for (TPair<int32, FFVMarker>& Pair : Markers)
	{
		if (UpdateMarker(Pair.Value))
		{
			Changed.Add(Pair.Value.Handle);
		}
	}

	for (const FFVMarkerHandle& Handle : Changed)
	{
		OnMarkerVisibilityChanged.Broadcast(Handle);
	}
}

void UFVNavigationSubsystem::SetTrackedMarker(FFVMarkerHandle Marker)
{
	const FFVMarkerHandle NewTracked = Markers.Contains(Marker.Id) ? Marker : FFVMarkerHandle();
	if (TrackedMarker == NewTracked)
	{
		return;
	}

	TrackedMarker = NewTracked;
	OnTrackedMarkerChanged.Broadcast(TrackedMarker);
}

FFVMarkerHandle UFVNavigationSubsystem::SetWaypoint(FVector Location)
{
	UFVMarkerDefinition* Definition = UFVNavigationSettings::Get().WaypointMarker.LoadSynchronous();
	if (Definition == nullptr)
	{
		UE_LOG(LogFVNavigationSystem, Warning, TEXT("SetWaypoint: no Waypoint Marker in Navigation settings."));
		return FFVMarkerHandle();
	}

	const FFVMarkerHandle Previous = Waypoint;
	Waypoint.Reset();
	RemoveMarker(Previous);

	Waypoint = AddMarkerAtLocation(Definition, Location, FText::GetEmpty());
	SetTrackedMarker(Waypoint);
	OnWaypointChanged.Broadcast(Waypoint);
	return Waypoint;
}

void UFVNavigationSubsystem::ClearWaypoint()
{
	RemoveMarker(Waypoint);
}

void UFVNavigationSubsystem::SetNavigator(UFVNavigatorComponent* InNavigator)
{
	Navigator = InNavigator;
	RefreshMarkers();
}

bool UFVNavigationSubsystem::UpdateMarker(FFVMarker& Marker)
{
	Marker.bDiscovered = ReadDiscovered(Marker);

	const bool bWasVisible = Marker.bVisible;
	const FFVConditionContext Context = MakeContext(Marker, Navigator.IsValid() ? Navigator->GetOwner() : nullptr);
	Marker.bVisible = Marker.Definition != nullptr && Marker.Definition->VisibleWhen.Evaluate(Context);
	return bWasVisible != Marker.bVisible;
}

bool UFVNavigationSubsystem::ReadDiscovered(const FFVMarker& Marker) const
{
	if (Marker.Definition == nullptr || Marker.Definition->FindFragment<FFVMarkerFragment_Discovery>() == nullptr)
	{
		return true;
	}

	const UFVMapMarkerComponent* Component = Marker.Component.Get();
	if (Component == nullptr || !Component->GetDiscoveredFact().IsValid())
	{
		return Marker.bDiscovered;
	}

	const UFVFactDatabase* Facts = UFVFactDatabase::Get(this);
	return Facts && Facts->GetFact(Component->GetDiscoveredFact()) > 0;
}

FFVConditionContext UFVNavigationSubsystem::MakeContext(const FFVMarker& Marker, AActor* Instigator) const
{
	FFVConditionContext Context;
	Context.WorldContext = const_cast<UFVNavigationSubsystem*>(this);
	Context.Instigator = Instigator;
	Context.Target = Marker.GetActor();
	return Context;
}

void UFVNavigationSubsystem::HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
	RefreshMarkers();
}
