#include "FVMapCaptureActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "EngineUtils.h"
#include "FVMapDefinition.h"
#include "FVNavigationSystem.h"
#include "FVNavigationTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMapCaptureActor)

static const FRotator TopDownRotation(-90.f, 0.f, 0.f);

#if WITH_EDITOR
AFVMapCaptureActor::FFVOnMapCaptureRequested AFVMapCaptureActor::OnCaptureRequested;
#endif

AFVMapCaptureActor::AFVMapCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;

	CaptureBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CaptureBox"));
	CaptureBox->SetBoxExtent(FVector(5000.f, 5000.f, 1000.f), false);
	CaptureBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CaptureBox->SetCanEverAffectNavigation(false);
	CaptureBox->SetUsingAbsoluteRotation(true);
	RootComponent = CaptureBox;

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->SetupAttachment(CaptureBox);
	CaptureComponent->SetUsingAbsoluteRotation(true);
	CaptureComponent->SetRelativeRotation(TopDownRotation);
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
}

FBox AFVMapCaptureActor::GetCaptureBounds() const
{
	const FVector Center = CaptureBox->GetComponentLocation();
	const FVector Extent = CaptureBox->GetScaledBoxExtent();
	return FBox(Center - Extent, Center + Extent);
}

FIntPoint AFVMapCaptureActor::GetCaptureSize() const
{
	const FVector Size = GetCaptureBounds().GetSize();
	const double Longest = FMath::Max3(Size.X, Size.Y, 1.0);

	// Image width runs along world Y (east), height along world X (north).
	auto RoundToBlock = [](double Pixels) { return FMath::Max(4, FMath::RoundToInt(Pixels / 4.0) * 4); };
	return FIntPoint(RoundToBlock(Resolution * Size.Y / Longest), RoundToBlock(Resolution * Size.X / Longest));
}

FBox2D AFVMapCaptureActor::GetCapturedArea() const
{
	const FBox Bounds = GetCaptureBounds();
	const FVector Center = Bounds.GetCenter();
	const FIntPoint Pixels = GetCaptureSize();

	const double Width = Bounds.GetSize().Y;
	const double Height = Width * Pixels.Y / FMath::Max(Pixels.X, 1);
	return FBox2D(
		FVector2D(Center.X - Height * 0.5, Center.Y - Width * 0.5),
		FVector2D(Center.X + Height * 0.5, Center.Y + Width * 0.5));
}

void AFVMapCaptureActor::PrepareCapture()
{
	const FBox Bounds = GetCaptureBounds();
	const FVector Center = Bounds.GetCenter();

	CaptureComponent->SetWorldLocationAndRotation(FVector(Center.X, Center.Y, Bounds.Max.Z), TopDownRotation);
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->OrthoWidth = Bounds.GetSize().Y;
	CaptureComponent->MaxViewDistanceOverride = bClipBelowBox ? Bounds.GetSize().Z : -1.f;

	TArray<AActor*> Ignored;
	GatherIgnoredActors(Ignored);

	CaptureComponent->HiddenActors.Reset(Ignored.Num());
	for (AActor* Actor : Ignored)
	{
		CaptureComponent->HiddenActors.Add(Actor);
	}
}

void AFVMapCaptureActor::GatherIgnoredActors(TArray<AActor*>& OutActors) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	TArray<UClass*> Classes;
	for (const TSubclassOf<AActor>& Class : Filter.IgnoredClasses)
	{
		if (Class != nullptr)
		{
			Classes.Add(Class);
		}
	}

	TArray<FName> IgnoredTags = Filter.IgnoredActorTags;

	if (Filter.bUseProjectDefaults)
	{
		const UFVNavigationSettings& Settings = UFVNavigationSettings::Get();
		for (const TSoftClassPtr<AActor>& Soft : Settings.CaptureIgnoredClasses)
		{
			if (UClass* Class = Soft.LoadSynchronous())
			{
				Classes.Add(Class);
			}
		}
		IgnoredTags.Append(Settings.CaptureIgnoredActorTags);
	}

	const FBox Bounds = GetCaptureBounds();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor == this)
		{
			OutActors.Add(Actor);
			continue;
		}

		const bool bIgnoredClass = Classes.ContainsByPredicate([Actor](const UClass* Class) { return Actor->IsA(Class); });
		const bool bIgnoredTag = IgnoredTags.ContainsByPredicate([Actor](const FName& Tag) { return Actor->ActorHasTag(Tag); });
		const bool bIgnoredActor = Filter.IgnoredActors.ContainsByPredicate([Actor](const TSoftObjectPtr<AActor>& Soft) { return Soft.Get() == Actor; });

		bool bAbove = false;
		if (Filter.bIgnoreActorsAboveBox)
		{
			const FBox ActorBounds = Actor->GetComponentsBoundingBox(true);
			bAbove = ActorBounds.IsValid && ActorBounds.Min.Z > Bounds.Max.Z;
		}

		if (bIgnoredClass || bIgnoredTag || bIgnoredActor || bAbove)
		{
			OutActors.Add(Actor);
		}
	}
}

TArray<FString> AFVMapCaptureActor::GetLayerNames() const
{
	TArray<FString> Names;
	if (Map != nullptr)
	{
		for (const FFVMapLayer& MapLayer : Map->Layers)
		{
			Names.Add(MapLayer.Name.ToString());
		}
	}
	return Names;
}

#if WITH_EDITOR
void AFVMapCaptureActor::CaptureMap()
{
	if (!OnCaptureRequested.IsBound())
	{
		UE_LOG(LogFVNavigationSystem, Warning, TEXT("%s: map capture needs the FVNavigationSystemEditor module."), *GetName());
		return;
	}
	OnCaptureRequested.Broadcast(this);
}
#endif
