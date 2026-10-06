#include "FVNavigationDebugSubsystem.h"

#include "FVDebugUtils.h"
#include "FVMapDefinition.h"
#include "FVMarkerDefinition.h"
#include "FVNavigationSubsystem.h"
#include "FVNavigatorComponent.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNavigationDebugSubsystem)

static TAutoConsoleVariable<bool> CVarNavigationDebugHUD(
	TEXT("FVCvar.Navigation.Debug.HUD"),
	false,
	TEXT("Show the active map, the player's map position and nearby markers on screen."));

static TAutoConsoleVariable<float> CVarNavigationDebugRange(
	TEXT("FVCvar.Navigation.Debug.Range"),
	10000.f,
	TEXT("Markers within this distance are listed by the navigation debug HUD."));

static FString DescribeMarker(const FFVMarker& Marker, const FVector& From)
{
	return FString::Printf(TEXT("#%d %s \"%s\" %.0fm%s%s"),
		Marker.Handle.Id,
		*GetNameSafe(Marker.Definition),
		*Marker.GetLabel().ToString(),
		FVector::Dist(From, Marker.GetLocation()) / 100.f,
		Marker.bVisible ? TEXT("") : TEXT(" hidden"),
		Marker.bDiscovered ? TEXT("") : TEXT(" undiscovered"));
}

bool UFVNavigationDebugSubsystem::IsEnabled() const
{
	return CVarNavigationDebugHUD.GetValueOnGameThread();
}

void UFVNavigationDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(this);
	const UFVNavigatorComponent* Navigator = UFVNavigatorComponent::Find(FVDebug::GetPlayerPawn(GetWorld()));
	if (Navigation == nullptr || Navigator == nullptr)
	{
		OutLines.Add(TEXT("No navigator on the player pawn."));
		return;
	}

	const UFVMapDefinition* Map = Navigator->GetActiveMap();
	const FFVMapLayer* Layer = Map ? Map->GetLayer(Navigator->GetActiveLayer()) : nullptr;
	const FVector Location = Navigator->GetViewLocation();

	OutLines.Add(FString::Printf(TEXT("Map %s, layer %s, UV %s, heading %.0f"),
		*GetNameSafe(Map),
		Layer ? *Layer->Name.ToString() : TEXT("-"),
		Layer ? *Layer->WorldToUV(Location).ToString() : TEXT("-"),
		FRotator::ClampAxis(Navigator->GetViewYaw())));

	OutLines.Add(FString::Printf(TEXT("%d markers, tracked #%d, waypoint #%d"),
		Navigation->GetMarkers().Num(), Navigation->GetTrackedMarker().Id, Navigation->GetWaypoint().Id));

	const float Range = CVarNavigationDebugRange.GetValueOnGameThread();
	for (const TPair<int32, FFVMarker>& Pair : Navigation->GetMarkers())
	{
		if (FVector::Dist(Location, Pair.Value.GetLocation()) <= Range)
		{
			OutLines.Add(TEXT("  ") + DescribeMarker(Pair.Value, Location));
		}
	}
}

static FAutoConsoleCommandWithWorld CmdListMarkers(
	TEXT("FV.Navigation.ListMarkers"),
	TEXT("FV.Navigation.ListMarkers - log every registered map marker."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		const UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(World);
		if (Navigation == nullptr)
		{
			return;
		}

		const APawn* Pawn = FVDebug::GetPlayerPawn(World);
		const FVector From = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
		for (const TPair<int32, FFVMarker>& Pair : Navigation->GetMarkers())
		{
			UE_LOG(LogFVDebug, Display, TEXT("%s"), *DescribeMarker(Pair.Value, From));
		}
	}));

static FAutoConsoleCommandWithWorld CmdDiscoverAll(
	TEXT("FV.Navigation.DiscoverAll"),
	TEXT("FV.Navigation.DiscoverAll - discover every marker that can be discovered."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(World))
		{
			for (const FFVMarkerHandle& Handle : Navigation->GetMarkerHandles())
			{
				Navigation->DiscoverMarker(Handle, FVDebug::GetPlayerPawn(World));
			}
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdWaypoint(
	TEXT("FV.Navigation.Waypoint"),
	TEXT("FV.Navigation.Waypoint <X> <Y> - set the waypoint at a world location; no arguments clears it."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(World);
		if (Navigation == nullptr)
		{
			return;
		}

		if (Args.Num() < 2)
		{
			Navigation->ClearWaypoint();
			return;
		}

		const APawn* Pawn = FVDebug::GetPlayerPawn(World);
		const float Z = Pawn ? Pawn->GetActorLocation().Z : 0.f;
		Navigation->SetWaypoint(FVector(FCString::Atof(*Args[0]), FCString::Atof(*Args[1]), Z));
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdTrack(
	TEXT("FV.Navigation.Track"),
	TEXT("FV.Navigation.Track <MarkerId> - track a marker by the id ListMarkers prints; no argument stops tracking."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (UFVNavigationSubsystem* Navigation = UFVNavigationSubsystem::Get(World))
		{
			FFVMarkerHandle Handle;
			Handle.Id = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : INDEX_NONE;
			Navigation->SetTrackedMarker(Handle);
		}
	}));
