#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractableComponent.h"
#include "Conditions/FVConditionStatics.h"
#include "Data/FVInteractorDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "Core/FVInteractionGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Input/Components/FVGestureComponent.h"

#if !UE_BUILD_SHIPPING
#include "Subsystems/FVInteractionDebugSubsystem.h"
#include "Facts/FVFactDatabase.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorComponent)

UFVInteractorComponent::UFVInteractorComponent()
	: State(EFVInteractorState::Idle)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

#if WITH_EDITOR
EDataValidationResult UFVInteractorComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (!Definition)
	{
		Context.AddError(FText::Format(NSLOCTEXT("FVInteractorComponent", "NoDefinition", "Interactor component '{0}' has no Definition assigned."), FText::FromString(GetName())));
		Result = EDataValidationResult::Invalid;
	}
	return Result;
}
#endif

void UFVInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureMsgf(Definition && Definition->DefaultMode, TEXT("%s on %s requires a Definition with a DefaultMode."), *GetName(), *GetNameSafe(GetOwner())))
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}

	GrantedTags = Definition->GrantedTags;
	BlockedActionTags = Definition->BlockedActionTags;

	ModeStack.Reset();
	FFVInteractorModeEntry& DefaultEntry = ModeStack.AddDefaulted_GetRef();
	DefaultEntry.Mode = Definition->DefaultMode;
	DefaultEntry.Priority = MIN_int32;
	DefaultEntry.PushOrder = NextPushOrder++;

	ActiveMode = Definition->DefaultMode;
	ActiveDetection = ActiveMode->Detection;
	BlendTarget = ActiveDetection;
	BlendDuration = 0.f;

	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		FactChangedHandle = Facts->OnFactChangedNative().AddUObject(this, &UFVInteractorComponent::HandleFactChanged);
	}
	
	if (!Cast<APawn>(GetOwner()))
	{
		UE_LOG(LogFVInteraction, Warning, TEXT("Interaction Component has invalid pawn owner."));
		return;
	}

	Registry = GetWorld()->GetSubsystem<UFVInteractionRegistrySubsystem>();
	
	if (IsValid(Registry))
	{
		Registry->RegisterInteractor(this);
	}
	
	SetComponentTickInterval(ActiveDetection.TickInterval);

#if !UE_BUILD_SHIPPING
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		DebugSubsystem = GameInstance->GetSubsystem<UFVInteractionDebugSubsystem>();
	}
#endif
}

void UFVInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickModeBlend(DeltaTime);

	if (State == EFVInteractorState::Interacting)
	{
		TickInteraction(DeltaTime);
		return;
	}

	if (State == EFVInteractorState::Idle || State == EFVInteractorState::Awake)
	{
		const bool bHasInteractablesInRange = IsValid(Registry) && !Registry->GetActiveInteractables().IsEmpty();
		SetState(bHasInteractablesInRange ? EFVInteractorState::Awake : EFVInteractorState::Idle);
	}
	
	if (State != EFVInteractorState::Awake)
	{
		if (TargetInteractable.IsValid())
		{
			ReleaseTargetInteractable();
			RefreshOffers();
		}
		return;
	}
	
	PerformTrace();
}

void UFVInteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->OnFactChangedNative().Remove(FactChangedHandle);
	}

	DisableTracing();

	if (IsValid(Registry))
	{
		Registry->UnregisterInteractor();
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInteractorComponent::EnableTracing()
{
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UFVInteractorComponent::DisableTracing()
{
	CancelInteraction(FGameplayTag());
	ReleaseTargetInteractable();
	RefreshOffers();
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UFVInteractorComponent::AddSuppression(const FGameplayTag Reason)
{
	if (!Reason.IsValid() || SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.AddTag(Reason);
	CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Suppressed);
	SetState(EFVInteractorState::Suppressed);
}

void UFVInteractorComponent::RemoveSuppression(const FGameplayTag Reason)
{
	if (!SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.RemoveTag(Reason);

	if (SuppressionReasons.IsEmpty() && State == EFVInteractorState::Suppressed)
	{
		EFVInteractorState NewState = EFVInteractorState::Idle;

		if (IsValid(Registry) && Registry->GetActiveInteractables().Num() > 0)
		{
			NewState = EFVInteractorState::Awake;
		}

		SetState(NewState);
	}
}

void UFVInteractorComponent::SetState(const EFVInteractorState NewState)
{
	if (State == NewState)
	{
		return;
	}
	
	State = NewState;
	StateChanged.Broadcast(State);
}

FGameplayTag UFVInteractorComponent::GetInteractorTag() const
{
	return Definition ? Definition->InteractorTag : FGameplayTag();
}

bool UFVInteractorComponent::PushMode(const FGameplayTag ModeTag, const int32 Priority)
{
	const UFVInteractorModeDefinition* Mode = Definition ? Definition->FindMode(ModeTag) : nullptr;
	if (!Mode)
	{
		UE_LOG(LogFVInteraction, Warning, TEXT("%s: interactor mode '%s' is not declared in the definition."), *GetNameSafe(GetOwner()), *ModeTag.ToString());
		return false;
	}

	if (Mode == Definition->DefaultMode)
	{
		return true;
	}

	FFVInteractorModeEntry* Entry = ModeStack.FindByPredicate([Mode](const FFVInteractorModeEntry& Candidate) { return Candidate.Mode == Mode; });
	if (!Entry)
	{
		Entry = &ModeStack.AddDefaulted_GetRef();
		Entry->Mode = Mode;
	}

	Entry->Priority = Priority;
	Entry->PushOrder = NextPushOrder++;
	ApplyTopMode();
	return true;
}

bool UFVInteractorComponent::RemoveMode(const FGameplayTag ModeTag)
{
	const int32 Removed = ModeStack.RemoveAll([&ModeTag](const FFVInteractorModeEntry& Entry)
	{
		return Entry.Mode && Entry.Priority != MIN_int32 && Entry.Mode->ModeTag.MatchesTagExact(ModeTag);
	});

	if (Removed > 0)
	{
		ApplyTopMode();
	}
	return Removed > 0;
}

void UFVInteractorComponent::ClearModes()
{
	ModeStack.RemoveAll([](const FFVInteractorModeEntry& Entry) { return Entry.Priority != MIN_int32; });
	ApplyTopMode();
}

bool UFVInteractorComponent::HasMode(const FGameplayTag ModeTag) const
{
	return ModeStack.ContainsByPredicate([&ModeTag](const FFVInteractorModeEntry& Entry)
	{
		return Entry.Mode && Entry.Mode->ModeTag.MatchesTagExact(ModeTag);
	});
}

FGameplayTag UFVInteractorComponent::GetActiveModeTag() const
{
	return ActiveMode ? ActiveMode->ModeTag : FGameplayTag();
}

const FFVInteractorModeEntry* UFVInteractorComponent::GetTopEntry() const
{
	const FFVInteractorModeEntry* Top = nullptr;
	for (const FFVInteractorModeEntry& Entry : ModeStack)
	{
		if (!Entry.Mode)
		{
			continue;
		}

		if (!Top || Entry.Priority > Top->Priority || (Entry.Priority == Top->Priority && Entry.PushOrder > Top->PushOrder))
		{
			Top = &Entry;
		}
	}
	return Top;
}

void UFVInteractorComponent::ApplyTopMode()
{
	const FFVInteractorModeEntry* Top = GetTopEntry();
	const UFVInteractorModeDefinition* NewMode = Top ? Top->Mode.Get() : nullptr;
	if (!NewMode || NewMode == ActiveMode)
	{
		return;
	}

	const FGameplayTag OldTag = GetActiveModeTag();
	ActiveMode = NewMode;

	BlendFrom = ActiveDetection;
	BlendTarget = NewMode->Detection;
	BlendElapsed = 0.f;
	BlendDuration = NewMode->BlendTime;

	if (BlendDuration <= 0.f)
	{
		ActiveDetection = BlendTarget;
	}
	else
	{
		ActiveDetection.CollisionChannel = BlendTarget.CollisionChannel;
		ActiveDetection.OcclusionChannel = BlendTarget.OcclusionChannel;
		ActiveDetection.TickInterval = BlendTarget.TickInterval;
		ActiveDetection.TraceOrigin = BlendTarget.TraceOrigin;
		ActiveDetection.bShowOverlay = BlendTarget.bShowOverlay;
		ActiveDetection.OverlayWidgetClass = BlendTarget.OverlayWidgetClass;
	}

	if (State != EFVInteractorState::Interacting)
	{
		SetComponentTickInterval(ActiveDetection.TickInterval);
	}

	InteractorModeChanged.Broadcast(NewMode->ModeTag, OldTag);
}

void UFVInteractorComponent::TickModeBlend(const float DeltaTime)
{
	if (BlendDuration <= 0.f)
	{
		return;
	}

	BlendElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(BlendElapsed / BlendDuration, 0.f, 1.f);
	ActiveDetection = FFVInteractorDetectionSettings::Lerp(BlendFrom, BlendTarget, Alpha);

	if (Alpha >= 1.f)
	{
		BlendDuration = 0.f;
	}
}

bool UFVInteractorComponent::BeginInteraction(const FGameplayTag InputTag)
{
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->DebugInput(InputTag, FPlatformTime::Seconds());
		Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::NoPrompt);
	}
#endif
	
	if (State != EFVInteractorState::Awake)
	{
		return false;
	}

	UFVInteractableComponent* Target = TargetInteractable.Get();
	if (!Target || !Target->CanInteract())
	{
		return false;
	}

	const FFVInteractionOfferData* Offer = FindOffer(InputTag);
	if (!Offer || !Offer->CanExecute())
	{
#if !UE_BUILD_SHIPPING
		if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
		{
			Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::Disabled);
		}
#endif
		return false;
	}

	UFVGestureComponent* Gesture = ResolveGestureComponent();
	if (!Gesture || !Gesture->BeginGesture(InputTag, Offer->Gesture))
	{
		return false;
	}

	Gesture->PushInput(InputTag, EFVInputPhase::Pressed);

	ActiveCommit = FFVInteractionCommit();
	ActiveCommit.ActionTag = Offer->ActionTag;
	ActiveCommit.InputTag = Offer->InputTag;
	ActiveCommit.Interactable = Target;
	ActiveCommit.Interactor = this;
	
	SetState(EFVInteractorState::Interacting);
	SetComponentTickInterval(0.f);
	Target->StartInteraction(ActiveCommit.ActionTag, this);
	InteractionCommitStarted.Broadcast(ActiveCommit);

	TickInteraction(0.f);
	return true;
}

void UFVInteractorComponent::TickInteraction(const float DeltaTime)
{
	UFVGestureComponent* Gesture = GestureComponent.Get();
	if (!Gesture || !ValidateActiveInteraction())
	{
		FinishInteraction(false);
		return;
	}

	const EFVGestureStatus Status = Gesture->UpdateGesture(DeltaTime);
	if (Status != EFVGestureStatus::Running)
	{
		FinishInteraction(Status == EFVGestureStatus::Completed);
		return;
	}

	const float Progress = Gesture->GetProgress();
	ActiveCommit.Interactable->ProgressInteraction(ActiveCommit.ActionTag, this, Progress);
	InteractionCommitProgressed.Broadcast(ActiveCommit, Progress);
}

bool UFVInteractorComponent::ValidateActiveInteraction() const
{
	const UFVInteractableComponent* Target = TargetInteractable.Get();
	if (!Target || Target != ActiveCommit.Interactable || !InteractableIsInReach(Target))
	{
		return false;
	}

	if (Target->GetState() != EFVInteractableState::Interacting)
	{
		return false;
	}

	const FFVInteractionOfferData* Offer = FindOffer(ActiveCommit.InputTag);
	return Offer && Offer->CanExecute();
}

void UFVInteractorComponent::CancelInteraction(const FGameplayTag Reason)
{
	if (State == EFVInteractorState::Interacting)
	{
		FinishInteraction(false);
	}
}

void UFVInteractorComponent::FinishInteraction(const bool bSuccess)
{
	if (UFVGestureComponent* Gesture = GestureComponent.Get())
	{
		Gesture->ResetGesture();
	}

#if !UE_BUILD_SHIPPING
	if (bSuccess)
	{
		if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
		{
			Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::Succeeded);
		}
	}
#endif

	const FFVInteractionCommit Commit = ActiveCommit;
	ActiveCommit = FFVInteractionCommit();

	SetState(EFVInteractorState::Awake);
	SetComponentTickInterval(ActiveDetection.TickInterval);
	
	if (UFVInteractableComponent* Target = Commit.Interactable)
	{
		if (bSuccess)
		{
			Target->ExecuteAction(Commit.ActionTag, this);
			Target->ConsumeOffer(Commit.InputTag);
		}

		Target->EndInteraction(Commit.ActionTag, this, bSuccess);
	}
	
	InteractionCommitEnded.Broadcast(Commit, bSuccess);
	RefreshOffers();
}

const FFVInteractionOfferData* UFVInteractorComponent::FindOffer(const FGameplayTag& InputTag) const
{
	return CachedOffers.FindByPredicate([&InputTag](const FFVInteractionOfferData& Candidate)
	{
		return Candidate.InputTag.MatchesTagExact(InputTag);
	});
}

UFVGestureComponent* UFVInteractorComponent::ResolveGestureComponent()
{
	if (UFVGestureComponent* Cached = GestureComponent.Get())
	{
		return Cached;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return nullptr;
	}

	UFVGestureComponent* Found = Pawn->FindComponentByClass<UFVGestureComponent>();
	if (!Found && Pawn->GetController())
	{
		Found = Pawn->GetController()->FindComponentByClass<UFVGestureComponent>();
	}

	ensureMsgf(Found, TEXT("%s requires a UFVGestureComponent on its pawn or controller."), *GetNameSafe(GetOwner()));
	GestureComponent = Found;
	return Found;
}

bool UFVInteractorComponent::IsOfferAvailable(const FFVInteractionOffer& Offer) const
{
	if (BlockedActionTags.HasTag(Offer.ActionTag))
	{
		return false;
	}

	if (Offer.Conditions.IsEmpty())
	{
		return true;
	}

	const UFVInteractableComponent* Target = TargetInteractable.Get();
	return Offer.Conditions.Evaluate(UFVConditionStatics::MakeContext(GetOwner(), Target ? Target->GetOwner() : nullptr));
}

void UFVInteractorComponent::HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
	if (!TargetInteractable.IsValid())
	{
		return;
	}
	RefreshOffers();
}

void UFVInteractorComponent::GrantTag(const FGameplayTag NewTag)
{
	if (!NewTag.IsValid() || GrantedTags.HasTagExact(NewTag))
	{
		return;
	}

	GrantedTags.AddTag(NewTag);
	RefreshOffers();
}

void UFVInteractorComponent::RemoveTag(const FGameplayTag OldTag)
{
	if (!GrantedTags.HasTagExact(OldTag))
	{
		return;
	}

	GrantedTags.RemoveTag(OldTag);
	RefreshOffers();
}

bool UFVInteractorComponent::PushInput(const FGameplayTag InputTag, const EFVInputPhase Phase)
{
	if (State == EFVInteractorState::Interacting)
	{
		UFVGestureComponent* Gesture = GestureComponent.Get();
		return Gesture && Gesture->PushInput(InputTag, Phase);
	}

	return Phase == EFVInputPhase::Pressed && BeginInteraction(InputTag);
}

void UFVInteractorComponent::PerformTrace()
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;

	if (!PC)
	{
		ReleaseTargetInteractable();
		RefreshOffers();

		return;
	}

	FVector PawnViewLocation;
	FRotator PawnViewRotation;
	Pawn->GetActorEyesViewPoint(PawnViewLocation, PawnViewRotation);
	
	FVector CameraViewLocation;
	FRotator CameraViewRotation;
	PC->GetPlayerViewPoint(CameraViewLocation, CameraViewRotation);

	FTraceData TraceData;
	{
		TraceData.CollisionChannel = ActiveDetection.CollisionChannel;
		TraceData.CollisionParams.AddIgnoredActor(Pawn);
		TraceData.CollisionParams.AddIgnoredActors(IgnoredActors);
		TraceData.CollisionParams.MobilityType = EQueryMobilityType::Any;
		TraceData.CollisionParams.bReturnPhysicalMaterial = true;
		
		if (ActiveDetection.TraceOrigin == EFVInteractorTraceOrigin::Camera)
		{
			const FVector Forward = CameraViewRotation.Vector();
			const float DistanceToPawn = FVector::DotProduct(PawnViewLocation - CameraViewLocation, Forward);

			TraceData.StartLocation = CameraViewLocation + Forward * FMath::Max(DistanceToPawn, 0.f);
			TraceData.TraceRotation = CameraViewRotation;
		}
		else
		{
			TraceData.StartLocation = PawnViewLocation;
			TraceData.TraceRotation = PawnViewRotation;
		}

		TraceData.StartLocation += TraceData.TraceRotation.RotateVector(ActiveDetection.TraceOffset);
		TraceData.EndLocation = TraceData.TraceRotation.Vector() * ActiveDetection.TraceRange + TraceData.StartLocation;
	}

	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ActiveDetection.TraceRadius);

	GetWorld()->SweepMultiByChannel
	(
		TraceData.HitResults,
		TraceData.StartLocation,
		TraceData.EndLocation,
		TraceData.TraceRotation.Quaternion(),
		TraceData.CollisionChannel,
		CollisionShape,
		TraceData.CollisionParams
	);

	UFVInteractableComponent* BestInteractableCandidate = nullptr;
	float BestScore = -1.f;
	
	for (FHitResult& HitResult : TraceData.HitResults)
	{	
		const AActor* HitActor = HitResult.GetActor();
		const UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		
		if (!IsValid(HitActor) || !IsValid(HitComponent))
			continue;
		
		UFVInteractableComponent* InteractableCandidate = HitActor->FindComponentByClass<UFVInteractableComponent>();
		
		if (!IsValid(InteractableCandidate))
			continue;
		if (!InteractableCandidate->CanInteract())
			continue;
		if (InteractableCandidate->GetCollisionChannel() != ActiveDetection.CollisionChannel)
			continue;
		if (!InteractableCandidate->GetDetectablePrimitives().Contains(HitComponent))
			continue;
		if (!InteractableCandidate->GetCompatibleInteractorTags().HasTag(GetInteractorTag()))
			continue;
			
		const FVector ViewDirection = CameraViewRotation.Vector();
		float CandidateScore = ScoreCandidate(InteractableCandidate, HitResult, CameraViewLocation, ViewDirection);

		// Small bias toward the current target so focus doesn't flicker between near-equal candidates.
		if (InteractableCandidate == TargetInteractable.Get())
		{
			CandidateScore *= 1.1f;
		}

		if (CandidateScore <= BestScore)
			continue;
		if (PerformOcclusionTest(TraceData.StartLocation, HitResult.ImpactPoint, HitActor))
			continue;
		
		BestScore = CandidateScore;
		BestInteractableCandidate = InteractableCandidate;
	}

	if (BestInteractableCandidate != TargetInteractable.Get())
	{
		ReleaseTargetInteractable();
		
		if (IsValid(BestInteractableCandidate))
		{
			TargetInteractable = BestInteractableCandidate;
			BestInteractableCandidate->AcquireInteractor(this);
			InteractableFound.Broadcast(BestInteractableCandidate);
		}

		RefreshOffers();
	}

#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->VisualizeTrace(GetWorld(), TraceData, ActiveDetection.TraceRadius, ActiveDetection.TickInterval);
		Debug->DebugTrace(TraceData.HitResults);
	}
#endif
}

float UFVInteractorComponent::ScoreCandidate(const UFVInteractableComponent* Candidate, const FHitResult& Hit, const FVector& ViewLocation, const FVector& ViewDirection) const
{
	// Point on the hit primitive's bounds nearest the camera's line of sight, so large objects
	// (doors, tables) count as aligned whenever the view passes through them.
	const FBox Bounds = Hit.GetComponent()->Bounds.GetBox();
	const FVector OnViewLine = FMath::ClosestPointOnInfiniteLine(ViewLocation, ViewLocation + ViewDirection, Bounds.GetCenter());
	const FVector Focus = Bounds.GetClosestPointTo(OnViewLine);

	const FVector ToFocus = (Focus - ViewLocation).GetSafeNormal();
	const float AngleDeg = ToFocus.IsNearlyZero()
		? 0.f
		: FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(ViewDirection, ToFocus), -1.f, 1.f)));

	const float Alignment = FMath::Clamp(1.f - AngleDeg / ActiveDetection.AlignmentAngle, 0.f, 1.f);
	const float Closeness = FMath::Clamp(1.f - Hit.Distance / ActiveDetection.TraceRange, 0.f, 1.f);

	// Detection weight scales the score, so a heavier interactable still wins unless it is clearly off to the side.
	return Candidate->GetDetectionWeight() * (1.f + ActiveDetection.AlignmentWeight * Alignment + ActiveDetection.DistanceWeight * Closeness);
}

bool UFVInteractorComponent::PerformOcclusionTest(const FVector& Start, const FVector& End, const AActor* Target) const
{
	FHitResult OcclusionHit;
	FCollisionQueryParams QueryParams;
	{
		QueryParams.AddIgnoredActor(GetOwner());
	}

	GetWorld()->LineTraceSingleByChannel(OcclusionHit, Start, End, ActiveDetection.OcclusionChannel, QueryParams);
	
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->DebugOcclusion(OcclusionHit);
	}
#endif
	
	return OcclusionHit.IsValidBlockingHit() && OcclusionHit.GetActor() != Target; 
}

void UFVInteractorComponent::ReleaseTargetInteractable()
{
	if (UFVInteractableComponent* Interactable = TargetInteractable.Get())
	{
		Interactable->ReleaseInteractor(this);
		InteractableLost.Broadcast(Interactable);
	}
	
	TargetInteractable.Reset();
}

void UFVInteractorComponent::RefreshOffers(bool bForceBroadcast)
{
	const UFVInteractableComponent* Target = TargetInteractable.Get();
	TArray<FFVInteractionOfferData> NewOffers;

	if (Target)
	{
		for (const FFVInteractionOffer& Offer : Target->GetOffers())
		{
			if (!Offer.IsValid())
				continue;

			const bool bAvailable = IsOfferAvailable(Offer);

			if (!bAvailable && Offer.HidesWhenUnavailable())
				continue;

			NewOffers.Add(FFVInteractionOfferData::From(Offer, bAvailable));
		}
	}

	NewOffers.Sort([](const FFVInteractionOfferData& A, const FFVInteractionOfferData& B)
	{
		if (A.Weight != B.Weight)
		{
			return A.Weight > B.Weight;
		}

		return A.InputTag.GetTagName().LexicalLess(B.InputTag.GetTagName());
	});

	const bool bChanged = NewOffers != CachedOffers;
	CachedOffers = MoveTemp(NewOffers);

	if (bChanged || bForceBroadcast)
	{
		OffersChanged.Broadcast(CachedOffers);
	}
}

bool UFVInteractorComponent::InteractableIsInReach(const UFVInteractableComponent* Target) const
{
	const AActor* TargetActor = Target->GetOwner();
	return TargetActor && FVector::DistSquared(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(ActiveDetection.TraceRange * 1.5f);
}
