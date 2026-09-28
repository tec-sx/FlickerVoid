#include "FVGestureComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVGestureComponent)

UFVGestureComponent::UFVGestureComponent()
	: bIgnoreTimeDilation(true)
	, MashDecayPerSecond(0.f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFVGestureComponent::TickComponent(
	float DeltaTime, 
	ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!ActiveGesture.IsSet())
	{
		SetComponentTickEnabled(false);
		return;
	}
	
	const float GestureDela = bIgnoreTimeDilation ? static_cast<float>(FApp::GetDeltaTime()) : DeltaTime;
	const EFVGestureStatus Status = ProgressGesture(GestureDela);
	
	GestureProgressed.Broadcast(ActiveGesture->InputTag, Progress);
	
	if (Status != EFVGestureStatus::Running)
	{
		EndGesture(Status == EFVGestureStatus::Completed);
	}
}

bool UFVGestureComponent::BeginGesture(const FFVGesture& InGesture)
{
	if (ActiveGesture.IsSet())
		return false;
	
	ActiveGesture = InGesture;
	Elapsed = 0.f;
	Progress = 0.f;
	Presses = 0.f;
	bHoldReleased = false;
	bMashCanceled = false;
	
	SetComponentTickEnabled(true);
	return true;
}

bool UFVGestureComponent::PushInput(const FGameplayTag InputTag, const EFVInputPhase Phase)
{
	if (!ActiveGesture.IsSet() || !ActiveGesture->InputTag.MatchesTagExact(InputTag))
		return false;
	
	HandleInput(Phase);
	return true;
}

void UFVGestureComponent::AbortGesture()
{
	if (ActiveGesture.IsSet())
		EndGesture(false);
}

void UFVGestureComponent::HandleInput(const EFVInputPhase Phase)
{
	if (ActiveGesture->Mode == EFVGestureMode::Hold)
	{
		bHoldReleased |= Phase != EFVInputPhase::Pressed;
	}
	
	if (ActiveGesture->Mode == EFVGestureMode::Mash)
	{
		Presses += Phase == EFVInputPhase::Pressed ? 1.f : 0.f;
		bMashCanceled |= Phase == EFVInputPhase::Cancelled;
	}
}

EFVGestureStatus UFVGestureComponent::ProgressGesture(const float DeltaTime)
{
	switch (ActiveGesture->Mode)
	{
	case EFVGestureMode::Press:
		return ProgressPress();
	case EFVGestureMode::Hold:
		return ProgressHold(DeltaTime);
	case EFVGestureMode::Mash:
		return ProgressMash(DeltaTime);
	default:
		return EFVGestureStatus::Failed;
	}
}

EFVGestureStatus UFVGestureComponent::ProgressPress()
{
	Progress = 1.f;
	return EFVGestureStatus::Completed;
}

EFVGestureStatus UFVGestureComponent::ProgressHold(const float DeltaTime)
{
	if (bHoldReleased)
		return EFVGestureStatus::Failed;
	
	Elapsed += DeltaTime;
	Progress = ActiveGesture->Duration > 0.f ? FMath::Min(Elapsed / ActiveGesture->Duration, 1.f) : 1.f;
	
	return Progress >= 1.f ? EFVGestureStatus::Completed : EFVGestureStatus::Running;
}

EFVGestureStatus UFVGestureComponent::ProgressMash(const float DeltaTime)
{
	if (bMashCanceled)
		return  EFVGestureStatus::Failed;
	
	Elapsed += DeltaTime;
	Presses = FMath::Max(Presses - MashDecayPerSecond * DeltaTime, 0.f);
	Progress = FMath::Min(Presses ? FMath::Max(ActiveGesture->PressCount, 1) : 1.f);
	
	if (Progress >= 1.f)
		return EFVGestureStatus::Completed;
	
	const bool bTimedOut = ActiveGesture->Duration > 0.f && Elapsed >= ActiveGesture->Duration;
	
	return bTimedOut ? EFVGestureStatus::Failed : EFVGestureStatus::Running;
}

void UFVGestureComponent::EndGesture(const bool bSuccess)
{
	const FGameplayTag EndedGestureTag = ActiveGesture->InputTag;
	
	ActiveGesture.Reset();
	SetComponentTickEnabled(false);
	GestureEnded.Broadcast(EndedGestureTag, bSuccess);
}


