#pragma once
#include "GameplayTagContainer.h"

#include "FVInputTypes.generated.h"

UENUM(BlueprintType, meta=(ScriptName="InputPhase"))
enum class EFVInputPhase : uint8
{
	Pressed		UMETA(DisplayName="Pressed", Tooltip="Input key was pressed this frame."),
	Released	UMETA(DisplayName="Released", Tooltip="Input key was released this frame."),
	Cancelled	UMETA(DisplayName="Cancelled", Tooltip="Input was aborted without a commit."),
};

UENUM(BlueprintType, meta=(ScriptName="GestureMode"))
enum class EFVGestureMode : uint8
{
	Press		UMETA(DisplayName="Press", Tooltip="Completes immediately on press."),
	Hold		UMETA(DisplayName="Hold", Tooltip="Completes after the key is held for InteractionPeriod."),
	Mash		UMETA(DisplayName="Mash", Tooltip="Completes after RequiredPresses within InteractionPeriod."),
	Hover		UMETA(DisplayName="Hover", Tooltip="Completes after being focused for InteractionPeriod, no key needed."),
	Automatic	UMETA(DisplayName="Automatic", Tooltip="Completes as soon as the offer becomes available."),
};

UENUM(BlueprintType, meta=(ScriptName="GestureStatus"))
enum class EFVGestureStatus : uint8
{
	Inactive	UMETA(DisplayName="Inactive", Tooltip="Gesture input waiting."),
	Running		UMETA(DisplayName="Running", Tooltip="Gesture input is still running."),
	Completed	UMETA(DisplayName="Completed", Tooltip="Gesture input completed successfully."),
	Failed		UMETA(DisplayName="Failed", Tooltip="Gesture input failed."),
};

USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVGesture
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Gestures"))
	EFVGestureMode Mode = EFVGestureMode::Press;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", Units="s", EditCondition="Mode == EFVGestureMode::Hold", EditConditionHides))
	float Duration = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", EditCondition="Mode == EFVGestureMode::Mash", EditConditionHides))
	int32 PressCount = 1;

	bool operator==(const FFVGesture& Other) const
	{
		return Mode == Other.Mode && Duration == Other.Duration && PressCount == Other.PressCount;
	}
};