// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FVInteractionDebugSubsystem.h"

#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractableComponent.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionDebugSubsystem)

#if !UE_BUILD_SHIPPING

static TAutoConsoleVariable<int> CVarInteractionDebugDraw(
	TEXT("FVCvar.Interaction.Debug.Draw"),
	0,
	TEXT("Draw interaction detection volumes, aim probes, cones and focus highlights. 0 - off, 1 - basic, 2 - detailed"));

static TAutoConsoleVariable<bool> CVarInteractionDebugHUD(
	TEXT("FVCvar.Interaction.Debug.HUD"),
	false,
	TEXT("Show an on-screen readout of interaction candidates and resolved prompts"));

static void DrawBoundingBox(const UWorld* World, const AActor* Actor, const FColor Color, const float Interval)
{
	FVector BoundsOrigin;
	FVector BoundsExtent;
	Actor->GetActorBounds(true, BoundsOrigin, BoundsExtent);
	
	DrawDebugBox(World, BoundsOrigin, BoundsExtent, Actor->GetActorQuat(), Color, false, Interval, 0, 0.5f);
}

static void GetViewPoint(AActor* Actor, FVector& OutPawnLocation, FVector& OutViewLocation, FVector& OutForward)
{
	if (!Actor)
	{
		OutPawnLocation = FVector::ZeroVector;
		OutViewLocation = FVector::ZeroVector;
		OutForward = FVector::ForwardVector;
		return;
	}

	OutPawnLocation = Actor->GetActorLocation();
	OutViewLocation = OutPawnLocation;
	OutForward = Actor->GetActorForwardVector();

	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(OutViewLocation, ViewRotation);
			OutForward = ViewRotation.Vector();
		}
	}
}

#pragma region Initialization

void UFVInteractionDebugSubsystem::Register(const TWeakObjectPtr<UFVInteractorComponent> InInteractor)
{
	if (!InInteractor.IsValid())
		return;
	if (InInteractor == InteractorPtr)
		return;
	
	InteractorPtr = InInteractor;
	
	if (!HUDDrawHandle.IsValid())
	{
		FDebugDrawDelegate DrawHUDDelegate = FDebugDrawDelegate::CreateUObject(this, &UFVInteractionDebugSubsystem::DrawHUD);
		HUDDrawHandle = UDebugDrawService::Register(TEXT("Game"), DrawHUDDelegate);
	}
	
	if (UFVInteractorComponent* Interactor = InteractorPtr.Get())
	{
		Interactor->OffersChanged.AddDynamic(this, &UFVInteractionDebugSubsystem::OnOffersChanged);
		Interactor->InteractionCommitStarted.AddDynamic(this, &UFVInteractionDebugSubsystem::OnInteractionCommitStarted);
		Interactor->InteractionCommitProgressed.AddDynamic(this, &UFVInteractionDebugSubsystem::OnInteractionProgressed);
	}
}

void UFVInteractionDebugSubsystem::Unregister()
{
	if (UFVInteractorComponent* Interactor = InteractorPtr.Get())
	{
		Interactor->InteractionCommitProgressed.RemoveDynamic(this, &UFVInteractionDebugSubsystem::OnInteractionProgressed);
		Interactor->InteractionCommitStarted.RemoveDynamic(this, &UFVInteractionDebugSubsystem::OnInteractionCommitStarted);
		Interactor->OffersChanged.RemoveDynamic(this, &UFVInteractionDebugSubsystem::OnOffersChanged);
	}
	
	if (HUDDrawHandle.IsValid())
	{
		UDebugDrawService::Unregister(HUDDrawHandle);
		HUDDrawHandle.Reset();
	}

	InteractorPtr.Reset();
}

#pragma endregion 

#pragma region Visualization

void UFVInteractionDebugSubsystem::VisualizeRange(
	const UWorld* World, 
	const UFVInteractorComponent* Interactor, 
	const float Radius, 
	TArray<TObjectPtr<UFVInteractableComponent>>& ActiveInteractables,
	const float Interval)
{
	const int32 DrawLevel = CVarInteractionDebugDraw.GetValueOnGameThread();
	if (DrawLevel == 0 || !Interactor || !Interactor->GetOwner())
	{
		return;
	}
	
	DrawDebugCircle(World, Interactor->GetOwner()->GetActorLocation(), Radius, 64, FColor(0, 128, 255), false, Interval, 0, 0.5f, FVector::ForwardVector, FVector::RightVector, false);
	
	if (DrawLevel < 2)
	{
		return;
	}

	for (const UFVInteractableComponent* Interactable : ActiveInteractables)
	{
		if (!IsValid(Interactable) || !Interactable->GetOwner())
		{
			continue;
		}

		const FColor Color = Interactor->GetTargetInteractable() == Interactable ? FColor::Green : FColor::Yellow;
		DrawBoundingBox(World, Interactable->GetOwner(), Color, Interval);
	}
}

void UFVInteractionDebugSubsystem::VisualizeTrace(
	const UWorld* World,
	const FTraceData& InTraceData,
	const float TraceRadius,
	const float Interval)
{
	if (CVarInteractionDebugDraw.GetValueOnGameThread() == 0)
	{
		return;
	}
	
	FVector DiscX;
	FVector DiscY;
	UKismetMathLibrary::GetForwardVector(InTraceData.TraceRotation).FindBestAxisVectors(DiscX, DiscY);
	
	bool bHasHit = false;
	
	for (const FHitResult& HitResult : InTraceData.HitResults)
	{
		if (HitResult.IsValidBlockingHit())
		{
			bHasHit = true;
			DrawDebugLine(World, InTraceData.StartLocation, HitResult.ImpactPoint, FColor::Green,false,Interval,0, 0.25f);
			DrawDebugCircle(World, HitResult.ImpactPoint, TraceRadius, 16, FColor::Green, false, Interval, 0, 0.5f, DiscX, DiscY, false);
		}
	}
	
	if (!bHasHit)
	{
		DrawDebugLine(World, InTraceData.StartLocation, InTraceData.EndLocation, FColor::Blue,false,Interval,0, 0.25f);
		DrawDebugCircle(World, InTraceData.EndLocation, TraceRadius, 16, FColor::Blue, false, Interval, 0, 0.5f, DiscX, DiscY, false);
	}
}

#pragma endregion 

#pragma region Debugging

void UFVInteractionDebugSubsystem::DebugTrace(const TArray<FHitResult>& InHitResults)
{
	HitResults.Reset();
	HitResults = InHitResults;
}

void UFVInteractionDebugSubsystem::DebugOcclusion(const FHitResult& HitResult)
{
	OcclusionHitResult.Reset();
	OcclusionHitResult = HitResult;
}

void UFVInteractionDebugSubsystem::DebugInput(const FGameplayTag& InputTag, const double InputTime)
{
	LastInputTag = InputTag;
	LastInputTime = InputTime;
}

void UFVInteractionDebugSubsystem::DebugInteractionOutcome(const EFVDebugInteractionOutcome& Outcome)
{
	LastInteractionOutcome = Outcome;
}

void UFVInteractionDebugSubsystem::OnOffersChanged(const TArray<FFVInteractionOffer>& Offers)
{
	AvailableOffers.Reset();
	AvailableOffers = Offers;
}

#pragma region Delegate Handlers

void UFVInteractionDebugSubsystem::OnInteractionCommitStarted(const FFVInteractionCommit& Commit)
{
	ActiveCommit = Commit;	
}

void UFVInteractionDebugSubsystem::OnInteractionProgressed(const FFVInteractionCommit& Commit, float Progress)
{
	InteractionProgress = Progress;
}

#pragma endregion

#pragma endregion 

void UFVInteractionDebugSubsystem::DrawHUD(UCanvas* Canvas, APlayerController* PC)
{
	const UFVInteractorComponent* Interactor = InteractorPtr.Get();
	
	if (!IsValid(Interactor))
		return;
	if (!CVarInteractionDebugHUD.GetValueOnGameThread())
		return;
	if (!Canvas)
		return;
	
	constexpr float LineHeight = 16.f;
	constexpr float X = 20.f;
	float Y = 20.f;
	const FLinearColor HeaderColor = FLinearColor::Yellow;
	const FLinearColor TextColor = FLinearColor::White;
	
	auto DrawLine = [&](const FString& Text, const FLinearColor& Color)
	{
		Canvas->SetDrawColor(Color.ToFColor(true));
		Canvas->DrawText(GEngine->GetSmallFont(), Text, X, Y);
		Y += LineHeight;
	};
	
	// Global
	const UFVInteractableComponent* FocusedInteractable = Interactor->GetTargetInteractable();
	const AActor* FocusedActor = FocusedInteractable ? FocusedInteractable->GetOwner() : nullptr;
	
	// Candidates
	{
		DrawLine(TEXT("-- Interaction Candidates --"), HeaderColor);
		
		for (const FHitResult& HitResult : HitResults)
		{
			const AActor* ActorCandidate = HitResult.GetActor();
			const UFVInteractableComponent* InteractableCandidate = ActorCandidate 
				? ActorCandidate->FindComponentByClass<UFVInteractableComponent>() 
				: nullptr;
			
			if (!IsValid(InteractableCandidate) || !InteractableCandidate->CanInteract())
			{
				continue;
			}
			
			const bool bIsFocused = InteractableCandidate == FocusedInteractable;
			const bool bIsCompatible = InteractableCandidate->GetCompatibleInteractorTags().HasTag(Interactor->InteractorTag);

			DrawLine(
				FString::Printf(
					TEXT("%s%s:  Distance=%.0f  State=%s  Weight=%d IsCompatible=%s"), 
					bIsFocused ? TEXT("* ") : TEXT("  "), 
					IsValid(ActorCandidate) ? *ActorCandidate->GetName() : TEXT("?"), 
					FVector::Dist(HitResult.ImpactPoint, Interactor->GetOwner()->GetActorLocation()), 
					*UEnum::GetDisplayValueAsText(InteractableCandidate->GetState()).ToString(), 
					InteractableCandidate->GetDetectionWeight(),
					bIsCompatible ? TEXT("Yes") : TEXT("No")),
				bIsFocused && bIsCompatible ? HeaderColor : TextColor);
		}
	}
	
	// Occlusion 
	{
		const AActor* OccluderActor = OcclusionHitResult.GetActor();
		const bool bIsOccluded = OcclusionHitResult.IsValidBlockingHit() && OccluderActor != FocusedActor;
		
		Y += LineHeight * 0.5f;
		DrawLine(FString::Printf(TEXT("Occlusion: gate=%s %s"),
			bIsOccluded ? TEXT("blocked") : TEXT("open"),
			bIsOccluded ? *OcclusionHitResult.GetActor()->GetName() : TEXT("  ")),
			TextColor);
	}
	
	// Interaction
	{
		if (Interactor->GetState() == EFVInteractorState::Interacting && ActiveCommit.ActionTag.IsValid())
		{
			DrawLine(
				FString::Printf(TEXT("Active: %s %.0f%%"), *ActiveCommit.ActionTag.ToString(), InteractionProgress * 100.f),
				HeaderColor);
		}

		Y += LineHeight * 0.5f;
		DrawLine(TEXT("-- Offers --"), HeaderColor);
		
		if (AvailableOffers.IsEmpty())
		{
			DrawLine(TEXT("None"), TextColor);
		}
		else
		{
			for (const FFVInteractionOffer& Offer : AvailableOffers)
			{
				const TCHAR* Status = TEXT("AVAILABLE");
				if (Offer.IsExhausted())
				{
					Status = TEXT("SPENT");
				}
				else if (!Offer.bRequirementsMet)
				{
					Status = Offer.RequirementGate == EFVInteractionGate::Hide ? TEXT("HIDDEN") : TEXT("GATED");
				}

				DrawLine(
					FString::Printf(TEXT("  [%s] %s %s Uses=%d"),
						*Offer.InputTag.ToString(),
						*Offer.ActionTag.ToString(),
						Status,
						Offer.RemainingUses),
					TextColor);
			}
		}
		
		Y += LineHeight * 0.5f;

		int32 RegisteredCount = 0;
		if (const UWorld* World = GetWorld())
		{
			if (const UFVInteractionRegistrySubsystem* Registry = World->GetSubsystem<UFVInteractionRegistrySubsystem>())
			{
				RegisteredCount = Registry->GetAllInteractables().Num();
			}
		}
		
		DrawLine(FString::Printf(TEXT("Focused=%s Candidates=%d Registered=%d"),
			IsValid(FocusedActor) ? *FocusedActor->GetName() : TEXT("none"),
			HitResults.Num(),
			RegisteredCount),
			HeaderColor);

		
		DrawLine(FString::Printf(TEXT("Interactor=%s"),
			*UEnum::GetDisplayValueAsText(Interactor->GetState()).ToString()),
			HeaderColor);
		
		if (LastInteractionOutcome != EFVDebugInteractionOutcome::None)
		{
			const TCHAR* OutcomeText = TEXT("");
			FLinearColor OutcomeColor = TextColor;

			switch (LastInteractionOutcome)
			{
			case EFVDebugInteractionOutcome::Succeeded:
				OutcomeText = TEXT("SUCCESS");
				OutcomeColor = FLinearColor::Green;
				break;
			case EFVDebugInteractionOutcome::Disabled:
				OutcomeText = TEXT("FAILED (requirement not met)");
				OutcomeColor = FLinearColor::Red;
				break;
			default:
				OutcomeText = TEXT("FAILED (no prompt)");
				OutcomeColor = FLinearColor::Red;
				break;
			}

			const double Age = FPlatformTime::Seconds() - LastInputTime;
			
			DrawLine(
				FString::Printf(TEXT("Last=%s %s (%.1fs ago)"), *LastInputTag.ToString(), OutcomeText, Age),
				Age < 2.0 ? OutcomeColor : TextColor);
		}
	}
}

#endif
