#pragma once
#include "GameplayTagContainer.h"

UENUM(BlueprintType, meta=(ScriptName="InputPhase"))
enum class EFVInputPhase : uint8
{
	Pressed		UMETA(DisplayName="Pressed", Tooltip="Input key was pressed this frame."),
	Released	UMETA(DisplayName="Released", Tooltip="Input key was released this frame."),
	Cancelled	UMETA(DisplayName="Cancelled", Tooltip="Input was aborted without a commit."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="GestureMode"))
enum class EFVGestureMode : uint8
{
	Press		UMETA(DisplayName="Press", Tooltip="Commits immediately on press."),
	Hold		UMETA(DisplayName="Hold", Tooltip="Commits after the key is held for InteractionPeriod."),
	Mash		UMETA(DisplayName="Mash", Tooltip="Commits after RequiredPresses within InteractionPeriod."),
	Hover		UMETA(DisplayName="Hover", Tooltip="Commits after being focused for InteractionPeriod, no key needed."),
	Automatic	UMETA(DisplayName="Automatic", Tooltip="Commits as soon as the offer becomes available."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="GestureStatus"))
enum class EFVGestureStatus : uint8
{
	Running		UMETA(DisplayName="Running", Tooltip="Gesture input is still running."),
	Completed	UMETA(DisplayName="Completed", Tooltip="Gesture input completed successfully."),
	Failed		UMETA(DisplayName="Failed", Tooltip="Gesture input failed."),
	Default		UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FFVGesture
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag InputTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Gestures"))
	EFVGestureMode Mode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", Units="s", EditCondition="Mode == EFVGestureMode::Hold"))
	float Duration = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", EditCondition="Mode == EFVGestureMode::Mash"))
	int32 PressCount = 1;
};