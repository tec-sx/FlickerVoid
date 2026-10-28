#pragma once

#include "CoreMinimal.h"
#include "FVAnimationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVAnimationLibrary.generated.h"

/** Left/right naming helpers using the side pairs from the Animation System settings. */
UCLASS()
class FVANIMATIONSYSTEM_API UFVAnimationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Animation|Mirror", meta = (ToolTip = "Side of a bone from its suffix, such as thigh_l."))
	static EFVBodySide GetBoneSide(FName Bone);

	UFUNCTION(BlueprintPure, Category = "FV|Animation|Mirror", meta = (ToolTip = "The bone with its side suffix swapped; None for a centre bone."))
	static FName GetMirrorBoneName(FName Bone);

	UFUNCTION(BlueprintPure, Category = "FV|Animation|Mirror", meta = (ToolTip = "Side of a pose or curve name from its whole side tokens, such as upperarm_l_up. Centre when it has none or both."))
	static EFVBodySide GetNameSide(FName Name);

	UFUNCTION(BlueprintPure, Category = "FV|Animation|Mirror", meta = (ToolTip = "The pose or curve name with every side token swapped; None when it has none."))
	static FName GetMirrorName(FName Name);
};
