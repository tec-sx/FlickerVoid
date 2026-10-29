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
	
	// A zero delta is the evaluation on the press frame, which adds no time.
	float GestureDelta = 0.f;
	if (DeltaTime > 0.f)
	{
		GestureDelta = bIgnoreTimeDilation ? static_cast<float>(FApp::GetDeltaTime()) : DeltaTime;
	}
	Elapsed += GestureDelta;
	
	switch (ActiveGesture->Mode)
	{
	case EFVGestureMode::Press:
	case EFVGestureMode::Automatic:
		return EvaluatePress();
	case EFVGestureMode::Hold:
		return bReleased ? EFVGestureStatus::Failed : EvaluateDuration();
	case EFVGestureMode::Hover:
		return EvaluateDuration();
	case EFVGestureMode::Mash:
		return EvaluateMash(GestureDelta);
	default:
		return EFVGestureStatus::Failed;
	}
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

EFVGestureStatus UFVGestureComponent::EvaluateDuration()
{
	Progress = ActiveGesture->Duration > 0.f ? FMath::Min(Elapsed / ActiveGesture->Duration, 1.f) : 1.f;
	return Progress >= 1.f ? EFVGestureStatus::Completed : EFVGestureStatus::Running;
}

EFVGestureStatus UFVGestureComponent::EvaluateMash(const float DeltaTime)
{
	Presses = FMath::Max(Presses - MashDecayPerSecond * DeltaTime, 0.f);
	const int32 PressCount = FMath::Max(ActiveGesture->PressCount, 1);
	Progress = FMath::Min(Presses / PressCount, 1.f);
	
	if (Progress >= 1.f)
	{
		return EFVGestureStatus::Completed;
	}
	
	const bool bTimedOut = ActiveGesture->Duration > 0.f && Elapsed >= ActiveGesture->Duration;
	return bTimedOut ? EFVGestureStatus::Failed : EFVGestureStatus::Running;
}


