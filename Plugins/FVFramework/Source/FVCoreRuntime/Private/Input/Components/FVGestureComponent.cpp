#include "Input/Components/FVGestureComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVGestureComponent)

UFVGestureComponent::UFVGestureComponent()
	: bIgnoreTimeDilation(true)
	, MashDecayPerSecond(0.f)
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFVGestureComponent::BeginGesture(const FGameplayTag InInputTag, const FFVGesture& InGesture)
{
	if (ActiveGesture.IsSet() || !InInputTag.IsValid())
	{
		return false;
	}
	
	ActiveInputTag = InInputTag;
	ActiveGesture = InGesture;
	Elapsed = 0.f;
	Progress = 0.f;
	Presses = 0.f;
	bReleased = false;
	bCanceled = false;
	
	return true;
}

bool UFVGestureComponent::PushInput(const FGameplayTag InputTag, const EFVInputPhase Phase)
{
	if (!ActiveGesture.IsSet() || !ActiveInputTag.MatchesTagExact(InputTag))
	{
		return false;
	}

	switch (Phase)
	{
	case EFVInputPhase::Pressed:
		Presses += 1.f;
		break;
	case EFVInputPhase::Released:
		bReleased = true;
		break;
	case EFVInputPhase::Cancelled:
		bCanceled = true;
		break;
	default:
		return false;
	}
	
	return true;
}

EFVGestureStatus UFVGestureComponent::UpdateGesture(const float DeltaTime)
{
	if (!ActiveGesture.IsSet())
		return EFVGestureStatus::Inactive;
	if (bCanceled)
		return EFVGestureStatus::Failed;
	if (GetWorld()->IsPaused())
		return EFVGestureStatus::Running;
	
	const float GestureDelta = bIgnoreTimeDilation ? static_cast<float>(FApp::GetDeltaTime()) : DeltaTime;
	Elapsed += GestureDelta;
	
	if (ActiveGesture->Mode == EFVGestureMode::Press)
		return EvaluatePress();
	if (ActiveGesture->Mode == EFVGestureMode::Hold)
		return EvaluateHold();
	if (ActiveGesture->Mode == EFVGestureMode::Mash)
		return EvaluateMash(GestureDelta);
	
	return EFVGestureStatus::Failed;
}

void UFVGestureComponent::ResetGesture()
{
	ActiveGesture.Reset();
	ActiveInputTag = FGameplayTag();
	Progress = 0.f;
}

EFVGestureStatus UFVGestureComponent::EvaluatePress()
{
	Progress = 1.f;
	return EFVGestureStatus::Completed;
}

EFVGestureStatus UFVGestureComponent::EvaluateHold()
{
	if (bReleased)
	{
		return EFVGestureStatus::Failed;
	}
	
	Progress = ActiveGesture->Duration > 0.f ? FMath::Min(Elapsed / ActiveGesture->Duration, 1.f) : 1.f;
	return Progress >= 1.f ? EFVGestureStatus::Completed : EFVGestureStatus::Running;
}

EFVGestureStatus UFVGestureComponent::EvaluateMash(const float DeltaTime)
{
	Presses = FMath::Max(Presses - MashDecayPerSecond * DeltaTime, 0.f);
	Progress = FMath::Min(Presses ? FMath::Max(ActiveGesture->PressCount, 1) : 1.f);
	
	if (Progress >= 1.f)
	{
		return EFVGestureStatus::Completed;
	}
	
	const bool bTimedOut = ActiveGesture->Duration > 0.f && Elapsed >= ActiveGesture->Duration;
	return bTimedOut ? EFVGestureStatus::Failed : EFVGestureStatus::Running;
}


