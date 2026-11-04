#include "FVInteractionDebugSubsystem.h"

#include "Input/Components/FVGestureComponent.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Core/FVInteractionGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Relevance/FVRelevanceComponent.h"
#include "Relevance/FVRelevanceSettings.h"
#include "Relevance/FVRelevanceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionDebugSubsystem)

static TAutoConsoleVariable<int32> CVarInteractionDebugDraw(
	TEXT("FVCvar.Interaction.Debug.Draw"),
	0,
	TEXT("Draw the player's interaction range, traces and nearby interactables. 0 - off, 1 - range and traces, 2 - also nearby interactables"));

static TAutoConsoleVariable<bool> CVarInteractionDebugHUD(
	TEXT("FVCvar.Interaction.Debug.HUD"),
	false,
	TEXT("Show an on-screen readout of the player's interaction candidates and offers"));

namespace
{
	const TCHAR* GetOutcomeText(const EFVDebugInteractionOutcome Outcome)
	{
		switch (Outcome)
		{
		case EFVDebugInteractionOutcome::Succeeded: return TEXT("SUCCESS");
		case EFVDebugInteractionOutcome::Disabled:  return TEXT("FAILED (requirement not met)");
		case EFVDebugInteractionOutcome::NoPrompt:  return TEXT("FAILED (no prompt)");
		default:                                    return TEXT("");
		}
	}

	const TCHAR* GetOfferStatus(const FFVInteractionOfferData& Offer)
	{
		if (Offer.RemainingUses == 0)
		{
			return TEXT("SPENT");
		}
		if (!Offer.bAvailable)
		{
			return TEXT("LOCKED");
		}
		return TEXT("AVAILABLE");
	}
}

void UFVInteractionDebugSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	PostActorTickHandle = FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &UFVInteractionDebugSubsystem::HandlePostActorTick);
}

void UFVInteractionDebugSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldPostActorTick.Remove(PostActorTickHandle);
	PostActorTickHandle.Reset();
	Super::Deinitialize();
}

bool UFVInteractionDebugSubsystem::IsEnabled() const
{
	return CVarInteractionDebugHUD.GetValueOnGameThread();
}

const UFVInteractorComponent* UFVInteractionDebugSubsystem::FindPlayerInteractor() const
{
	const APawn* Pawn = FVDebug::GetPlayerPawn(GetWorld());
	return Pawn != nullptr ? Pawn->FindComponentByClass<UFVInteractorComponent>() : nullptr;
}

void UFVInteractionDebugSubsystem::HandlePostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds) const
{
	const int32 DrawLevel = CVarInteractionDebugDraw.GetValueOnGameThread();
	if (World != GetWorld() || DrawLevel <= 0)
	{
		return;
	}

	const UFVInteractorComponent* Interactor = FindPlayerInteractor();
	if (Interactor == nullptr || Interactor->GetOwner() == nullptr)
	{
		return;
	}

	DrawRange(*Interactor, DrawLevel);
	if (Interactor->GetState() == EFVInteractorState::Awake)
	{
		DrawTrace(*Interactor);
	}
}

void UFVInteractionDebugSubsystem::DrawRange(const UFVInteractorComponent& Interactor, const int32 DrawLevel) const
{
	const UWorld* World = GetWorld();
	const UFVRelevanceSettings& Settings = UFVRelevanceSettings::Get();
	if (const FFVRelevanceGroup* Group = Settings.FindGroup(FVInteractionGameplayTags::Relevance_Interactable))
	{
		const FVector Center = Interactor.GetOwner()->GetActorLocation();
		DrawDebugCircle(World, Center, Group->NearDistance, 64, FColor(0, 128, 255), false, -1.f, 0, 0.5f, FVector::ForwardVector, FVector::RightVector, false);
	}

	const UFVRelevanceSubsystem* Relevance = UFVRelevanceSubsystem::Get(World);
	if (DrawLevel < 2 || Relevance == nullptr)
	{
		return;
	}

	TArray<UFVRelevanceComponent*> NearComponents;
	Relevance->GetNearComponents(FVInteractionGameplayTags::Relevance_Interactable, NearComponents);

	const UFVInteractableComponent* Target = Interactor.GetTargetInteractable();
	const AActor* TargetActor = Target != nullptr ? Target->GetOwner() : nullptr;

	for (const UFVRelevanceComponent* Near : NearComponents)
	{
		const AActor* NearActor = Near->GetOwner();
		if (NearActor == nullptr)
		{
			continue;
		}

		FVector Origin;
		FVector Extent;
		NearActor->GetActorBounds(true, Origin, Extent);
		const FColor Color = NearActor == TargetActor ? FColor::Green : FColor::Yellow;
		DrawDebugBox(World, Origin, Extent, NearActor->GetActorQuat(), Color, false, -1.f, 0, 0.5f);
	}
}

void UFVInteractionDebugSubsystem::DrawTrace(const UFVInteractorComponent& Interactor) const
{
	const UWorld* World = GetWorld();
	const FTraceData& Trace = Interactor.GetDebugState().Trace;
	const float Radius = Interactor.GetActiveDetection().TraceRadius;

	const FVector Forward = Trace.TraceRotation.Vector();
	FVector DiscX;
	FVector DiscY;
	Forward.FindBestAxisVectors(DiscX, DiscY);

	DrawDebugSphere(World, Trace.StartLocation, 2.f, 8, FColor::Yellow, false, -1.f, 0, 0.f);

	bool bHasHit = false;
	for (const FHitResult& Hit : Trace.HitResults)
	{
		if (Hit.IsValidBlockingHit())
		{
			bHasHit = true;
			DrawDebugLine(World, Trace.StartLocation, Hit.ImpactPoint, FColor::Green, false, -1.f, 0, 0.25f);
			DrawDebugCircle(World, Hit.ImpactPoint, Radius, 16, FColor::Green, false, -1.f, 0, 0.5f, DiscX, DiscY, false);
		}
	}

	if (!bHasHit)
	{
		DrawDebugLine(World, Trace.StartLocation, Trace.EndLocation, FColor::Blue, false, -1.f, 0, 0.25f);
		DrawDebugCircle(World, Trace.EndLocation, Radius, 16, FColor::Blue, false, -1.f, 0, 0.5f, DiscX, DiscY, false);
	}
}

void UFVInteractionDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const UFVInteractorComponent* Interactor = FindPlayerInteractor();
	if (Interactor == nullptr)
	{
		OutLines.Add(TEXT("no interactor on the player pawn"));
		return;
	}

	const FFVInteractorDebugState& Debug = Interactor->GetDebugState();
	const UFVInteractableComponent* Focused = Interactor->GetTargetInteractable();
	const AActor* FocusedActor = Focused != nullptr ? Focused->GetOwner() : nullptr;
	const FVector InteractorLocation = Interactor->GetOwner()->GetActorLocation();
	const FGameplayTag InteractorTag = Interactor->GetInteractorTag();

	OutLines.Add(TEXT("-- Candidates --"));
	for (const FHitResult& Hit : Debug.Trace.HitResults)
	{
		const AActor* Candidate = Hit.GetActor();
		const UFVInteractableComponent* Interactable = Candidate != nullptr ? Candidate->FindComponentByClass<UFVInteractableComponent>() : nullptr;
		if (!IsValid(Interactable) || !Interactable->CanInteract())
		{
			continue;
		}

		const bool bFocused = Interactable == Focused;
		const FGameplayTagContainer& Compatible = Interactable->GetCompatibleInteractorTags();
		const bool bCompatible = Compatible.HasTag(InteractorTag);
		const FString StateName = UEnum::GetDisplayValueAsText(Interactable->GetState()).ToString();
		OutLines.Add(FString::Printf(TEXT("%s%s: Distance=%.0f State=%s Weight=%d Compatible=%s"),
			bFocused ? TEXT("* ") : TEXT("  "),
			*Candidate->GetName(),
			FVector::Dist(Hit.ImpactPoint, InteractorLocation),
			*StateName,
			Interactable->GetDetectionWeight(),
			bCompatible ? TEXT("Yes") : TEXT("No")));
	}

	const AActor* Occluder = Debug.OcclusionHit.GetActor();
	const bool bOccluded = Debug.OcclusionHit.IsValidBlockingHit() && Occluder != FocusedActor;
	OutLines.Add(FString::Printf(TEXT("Occlusion: %s %s"), bOccluded ? TEXT("blocked by") : TEXT("open"), bOccluded ? *GetNameSafe(Occluder) : TEXT("")));

	const FFVInteractionCommit& Commit = Interactor->GetActiveCommit();
	if (Interactor->GetState() == EFVInteractorState::Interacting && Commit.ActionTag.IsValid())
	{
		const UFVGestureComponent* Gesture = Interactor->GetOwner()->FindComponentByClass<UFVGestureComponent>();
		const float Progress = Gesture != nullptr ? Gesture->GetProgress() : 0.f;
		OutLines.Add(FString::Printf(TEXT("Active: %s %.0f%%"), *Commit.ActionTag.ToString(), Progress * 100.f));
	}

	OutLines.Add(TEXT("-- Offers --"));
	const TArray<FFVInteractionOfferData>& Offers = Interactor->GetOffers();
	if (Offers.IsEmpty())
	{
		OutLines.Add(TEXT("  none"));
	}
	for (const FFVInteractionOfferData& Offer : Offers)
	{
		OutLines.Add(FString::Printf(TEXT("  [%s] %s %s Uses=%d"), *Offer.InputTag.ToString(), *Offer.ActionTag.ToString(), GetOfferStatus(Offer), Offer.RemainingUses));
	}

	TArray<UFVRelevanceComponent*> NearComponents;
	if (const UFVRelevanceSubsystem* Relevance = UFVRelevanceSubsystem::Get(GetWorld()))
	{
		Relevance->GetNearComponents(FVInteractionGameplayTags::Relevance_Interactable, NearComponents);
	}

	const FString StateName = UEnum::GetDisplayValueAsText(Interactor->GetState()).ToString();
	OutLines.Add(FString::Printf(TEXT("Interactor=%s Focused=%s Candidates=%d InRange=%d"),
		*StateName, *GetNameSafe(FocusedActor), Debug.Trace.HitResults.Num(), NearComponents.Num()));

	if (Debug.LastOutcome != EFVDebugInteractionOutcome::None)
	{
		const double Age = FPlatformTime::Seconds() - Debug.LastInputTime;
		OutLines.Add(FString::Printf(TEXT("Last=%s %s (%.1fs ago)"), *Debug.LastInputTag.ToString(), GetOutcomeText(Debug.LastOutcome), Age));
	}
}
