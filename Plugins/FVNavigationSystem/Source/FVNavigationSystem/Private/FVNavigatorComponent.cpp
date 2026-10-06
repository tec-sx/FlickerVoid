#include "FVNavigatorComponent.h"

#include "FVMapDefinition.h"
#include "FVMarkerDefinition.h"
#include "FVNavigationSubsystem.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNavigatorComponent)

namespace FVNavigator
{
	static EFVMarkerElevation GetElevation(float DeltaZ)
	{
		const float Threshold = UFVNavigationSettings::Get().ElevationThreshold;
		if (DeltaZ > Threshold)
		{
			return EFVMarkerElevation::Above;
		}
		return DeltaZ < -Threshold ? EFVMarkerElevation::Below : EFVMarkerElevation::Level;
	}

	static void SortByPriority(TArray<FFVMarkerView>& Markers)
	{
		Markers.StableSort([](const FFVMarkerView& A, const FFVMarkerView& B)
		{
			const int32 PriorityA = A.Definition ? A.Definition->Priority : 0;
			const int32 PriorityB = B.Definition ? B.Definition->Priority : 0;
			return PriorityA < PriorityB;
		});
	}
}

UFVNavigatorComponent::UFVNavigatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

UFVNavigatorComponent* UFVNavigatorComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVNavigatorComponent>() : nullptr;
}

void UFVNavigatorComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickInterval(UFVNavigationSettings::Get().UpdateInterval);

	if (UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this))
	{
		Navigation->SetNavigator(this);
	}
	UpdateNavigation();
}

void UFVNavigatorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	if (Navigation && Navigation->GetNavigator() == this)
	{
		Navigation->SetNavigator(nullptr);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVNavigatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateNavigation();
}

FVector UFVNavigatorComponent::GetViewLocation() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}

float UFVNavigatorComponent::GetViewYaw() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->GetController())
	{
		return Pawn->GetControlRotation().Yaw;
	}
	return GetOwnerYaw();
}

float UFVNavigatorComponent::GetOwnerYaw() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorRotation().Yaw : 0.f;
}

void UFVNavigatorComponent::SetMinimapRadius(float Radius)
{
	Minimap.Radius = FMath::Clamp(Radius, Minimap.MinRadius, Minimap.MaxRadius);
}

void UFVNavigatorComponent::ZoomMinimap(float Factor)
{
	SetMinimapRadius(Minimap.Radius * Factor);
}

void UFVNavigatorComponent::SetCategoryHidden(FGameplayTag Category, bool bHidden)
{
	if (bHidden)
	{
		HiddenCategories.AddTag(Category);
	}
	else
	{
		HiddenCategories.RemoveTag(Category);
	}
}

void UFVNavigatorComponent::UpdateNavigation()
{
	UpdateActiveMap();
	DiscoverNearbyMarkers();
}

void UFVNavigatorComponent::UpdateActiveMap()
{
	const UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	if (Navigation == nullptr)
	{
		return;
	}

	int32 LayerIndex = INDEX_NONE;
	UFVMapDefinition* Map = Navigation->FindMapAt(GetViewLocation(), LayerIndex);
	if (Map == ActiveMap && LayerIndex == ActiveLayer)
	{
		return;
	}

	ActiveMap = Map;
	ActiveLayer = LayerIndex;
	OnActiveMapChanged.Broadcast(ActiveMap, ActiveLayer);
}

void UFVNavigatorComponent::DiscoverNearbyMarkers()
{
	UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	if (Navigation == nullptr)
	{
		return;
	}

	const FVector Location = GetViewLocation();
	TArray<FFVMarkerHandle> Found;

	for (const TPair<int32, FFVMarker>& Pair : Navigation->GetMarkers())
	{
		const FFVMarker& Marker = Pair.Value;
		if (Marker.bDiscovered || !Marker.bVisible)
		{
			continue;
		}

		const FFVMarkerFragment_Discovery* Discovery = Marker.Definition->FindFragment<FFVMarkerFragment_Discovery>();
		if (Discovery && FVector::DistSquared(Location, Marker.GetLocation()) <= FMath::Square(Discovery->Radius))
		{
			Found.Add(Marker.Handle);
		}
	}

	for (const FFVMarkerHandle& Handle : Found)
	{
		Navigation->DiscoverMarker(Handle, GetOwner());
	}
}

bool UFVNavigatorComponent::PassesFilters(const FFVMarker& Marker, bool bTracked) const
{
	if (!Marker.bVisible || Marker.Definition == nullptr)
	{
		return false;
	}

	if (!bTracked && Marker.Definition->Tags.HasAny(HiddenCategories))
	{
		return false;
	}

	const FFVMarkerFragment_Discovery* Discovery = Marker.Definition->FindFragment<FFVMarkerFragment_Discovery>();
	return Marker.bDiscovered || Discovery == nullptr || !Discovery->bHiddenUntilDiscovered;
}

void UFVNavigatorComponent::FillView(FFVMarkerView& View, const FFVMarker& Marker, const FVector& ViewLocation, bool bTracked) const
{
	const FVector Location = Marker.GetLocation();
	View.Handle = Marker.Handle;
	View.Definition = Marker.Definition;
	View.Label = Marker.GetLabel();
	View.Distance = FVector::Dist(ViewLocation, Location);
	View.Elevation = FVNavigator::GetElevation(Location.Z - ViewLocation.Z);
	View.bDiscovered = Marker.bDiscovered;
	View.bTracked = bTracked;
}

FFVMinimapView UFVNavigatorComponent::BuildMinimapView() const
{
	FFVMinimapView View;
	View.Map = ActiveMap;
	View.LayerIndex = ActiveLayer;

	const FVector Center = GetViewLocation();
	const float MapYaw = Minimap.bRotateWithView ? GetViewYaw() : 0.f;
	const float Radius = FMath::Max(Minimap.Radius, 1.f);
	View.MapRotation = -MapYaw;
	View.PlayerRotation = GetOwnerYaw() - MapYaw;

	if (const FFVMapLayer* Layer = ActiveMap ? ActiveMap->GetLayer(ActiveLayer) : nullptr)
	{
		const FVector2D Size = Layer->GetWorldSize();
		View.CenterUV = Layer->WorldToUV(Center);
		View.ExtentUV = FVector2D(Radius / FMath::Max(Size.Y, 1.), Radius / FMath::Max(Size.X, 1.));
	}

	const UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	if (Navigation == nullptr)
	{
		return View;
	}

	float Sin = 0.f;
	float Cos = 1.f;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(MapYaw));

	for (const TPair<int32, FFVMarker>& Pair : Navigation->GetMarkers())
	{
		const FFVMarker& Marker = Pair.Value;
		const bool bTracked = Marker.Handle == Navigation->GetTrackedMarker();
		const FFVMarkerFragment_Minimap* Fragment = Marker.Definition ? Marker.Definition->FindFragment<FFVMarkerFragment_Minimap>() : nullptr;
		if (Fragment == nullptr || !PassesFilters(Marker, bTracked))
		{
			continue;
		}

		const FVector Delta = Marker.GetLocation() - Center;
		if (!bTracked && Fragment->MaxDistance > 0.f && Delta.Size2D() > Fragment->MaxDistance)
		{
			continue;
		}

		const float Forward = Delta.X * Cos + Delta.Y * Sin;
		const float Right = Delta.Y * Cos - Delta.X * Sin;
		FVector2D Position(Right / Radius, -Forward / Radius);

		bool bClamped = false;
		if (Position.SizeSquared() > 1.f)
		{
			if (!bTracked && !Fragment->bClampToEdge)
			{
				continue;
			}
			Position = Position.GetSafeNormal();
			bClamped = true;
		}

		FFVMarkerView& MarkerView = View.Markers.AddDefaulted_GetRef();
		FillView(MarkerView, Marker, Center, bTracked);
		MarkerView.Position = Position;
		MarkerView.bClamped = bClamped;
		MarkerView.Rotation = Fragment->bRotateWithActor ? Marker.GetYaw() - MapYaw : 0.f;
	}

	FVNavigator::SortByPriority(View.Markers);
	return View;
}

FFVCompassView UFVNavigatorComponent::BuildCompassView() const
{
	FFVCompassView View;

	const FVector Center = GetViewLocation();
	const float ViewYaw = GetViewYaw();
	const float HalfFieldOfView = Compass.FieldOfView * 0.5f;
	View.Heading = FRotator::ClampAxis(ViewYaw);

	const UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	if (Navigation == nullptr)
	{
		return View;
	}

	for (const TPair<int32, FFVMarker>& Pair : Navigation->GetMarkers())
	{
		const FFVMarker& Marker = Pair.Value;
		const bool bTracked = Marker.Handle == Navigation->GetTrackedMarker();
		const FFVMarkerFragment_Compass* Fragment = Marker.Definition ? Marker.Definition->FindFragment<FFVMarkerFragment_Compass>() : nullptr;
		if (Fragment == nullptr || !PassesFilters(Marker, bTracked))
		{
			continue;
		}

		const FVector Delta = Marker.GetLocation() - Center;
		if (!bTracked && Fragment->MaxDistance > 0.f && Delta.Size2D() > Fragment->MaxDistance)
		{
			continue;
		}

		const float Bearing = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
		float Relative = FRotator::NormalizeAxis(Bearing - ViewYaw);

		bool bClamped = false;
		if (FMath::Abs(Relative) > HalfFieldOfView)
		{
			if (!bTracked)
			{
				continue;
			}
			Relative = FMath::Sign(Relative) * HalfFieldOfView;
			bClamped = true;
		}

		FFVMarkerView& MarkerView = View.Markers.AddDefaulted_GetRef();
		FillView(MarkerView, Marker, Center, bTracked);
		MarkerView.Position = FVector2D(Relative / HalfFieldOfView, 0.f);
		MarkerView.Rotation = Relative;
		MarkerView.bClamped = bClamped;
	}

	FVNavigator::SortByPriority(View.Markers);
	return View;
}

FFVWorldMapView UFVNavigatorComponent::BuildWorldMapView(UFVMapDefinition* Map, int32 LayerIndex, float Zoom) const
{
	FFVWorldMapView View;
	View.Map = Map;
	View.LayerIndex = LayerIndex;

	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	const UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	if (Layer == nullptr || Navigation == nullptr)
	{
		return View;
	}

	const FVector PlayerLocation = GetViewLocation();
	View.bPlayerOnLayer = Layer->Contains(PlayerLocation);
	View.PlayerUV = Layer->WorldToUV(PlayerLocation);
	View.PlayerRotation = GetOwnerYaw();

	for (const TPair<int32, FFVMarker>& Pair : Navigation->GetMarkers())
	{
		const FFVMarker& Marker = Pair.Value;
		const bool bTracked = Marker.Handle == Navigation->GetTrackedMarker();
		const FFVMarkerFragment_WorldMap* Fragment = Marker.Definition ? Marker.Definition->FindFragment<FFVMarkerFragment_WorldMap>() : nullptr;
		if (Fragment == nullptr || !PassesFilters(Marker, bTracked) || (!bTracked && Zoom < Fragment->MinZoom))
		{
			continue;
		}

		const FVector Location = Marker.GetLocation();
		if (!Layer->Contains(Location) && !(bTracked && Layer->ContainsXY(Location)))
		{
			continue;
		}

		FFVMarkerView& MarkerView = View.Markers.AddDefaulted_GetRef();
		FillView(MarkerView, Marker, PlayerLocation, bTracked);
		MarkerView.Position = Layer->WorldToUV(Location);
		MarkerView.Rotation = Fragment->bRotateWithActor ? Marker.GetYaw() : 0.f;
	}

	FVNavigator::SortByPriority(View.Markers);
	return View;
}
