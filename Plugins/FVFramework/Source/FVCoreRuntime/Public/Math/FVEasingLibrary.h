#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVEasingLibrary.generated.h"

UENUM(BlueprintType)
enum class EFVEasing : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,
	SmoothStep,
	ExpoIn,
	ExpoOut,
	ExpoInOut,
	CircIn,
	CircOut,
	CircInOut
};

UCLASS()
class FVCORERUNTIME_API UFVEasingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Easing")
	static float Ease(float Alpha, EFVEasing Easing, float Exponent = 2.f);

	UFUNCTION(BlueprintPure, Category = "FV|Easing")
	static EFVEasing EasingFromName(FName Name);
};
