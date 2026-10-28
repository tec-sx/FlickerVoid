#pragma once

#include "CoreMinimal.h"
#include "FVAnimationTypes.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "BodySide"))
enum class EFVBodySide : uint8
{
	Centre UMETA(ToolTip = "No side suffix or token; never mirrored."),
	Left UMETA(ToolTip = "Carries the left side suffix or token, such as _l."),
	Right UMETA(ToolTip = "Carries the right side suffix or token, such as _r."),
};

UENUM(BlueprintType, meta = (ScriptName = "MirrorAxis"))
enum class EFVMirrorAxis : uint8
{
	Default UMETA(ToolTip = "The Default Mirror Axis from the Animation System settings."),
	X UMETA(ToolTip = "Mirror across the component-space YZ plane."),
	Y UMETA(ToolTip = "Mirror across the component-space XZ plane."),
	Z UMETA(ToolTip = "Mirror across the component-space XY plane."),
};

/** A left/right naming pair, such as _l and _r. */
USTRUCT(BlueprintType)
struct FVANIMATIONSYSTEM_API FFVSidePair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Side", meta = (ToolTip = "Left suffix, such as _l. Matched ignoring case; the case of the match is kept."))
	FString Left = TEXT("_l");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Side", meta = (ToolTip = "Right suffix, such as _r. Matched ignoring case; the case of the match is kept."))
	FString Right = TEXT("_r");
};
