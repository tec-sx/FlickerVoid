#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVAnimationTypes.h"
#include "FVAnimationSettings.generated.h"

/** Project settings for the animation system and the Meatbag Factory pose editor. */
UCLASS(Config = Game, DefaultConfig, NotBlueprintable, meta = (DisplayName = "Animation System"))
class FVANIMATIONSYSTEM_API UFVAnimationSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFVAnimationSettings(const FObjectInitializer& ObjectInitializer);

	static const UFVAnimationSettings& Get() { return *GetDefault<UFVAnimationSettings>(); }

	UPROPERTY(Config, EditAnywhere, Category = "Mirroring", meta = (ToolTip = "Suffixes that mark a bone's side. A bone without one is a centre bone and is never mirrored."))
	TArray<FFVSidePair> SidePairs;

	UPROPERTY(Config, EditAnywhere, Category = "Mirroring", meta = (ToolTip = "Default component-space axis that poses are mirrored across. Each mirror can pick another."))
	TEnumAsByte<EAxis::Type> MirrorAxis = EAxis::X;

	UPROPERTY(Config, EditAnywhere, Category = "Changed Bones", meta = (ClampMin = 0, Units = "deg", ToolTip = "A bone counts as changed by a pose when its rotation differs from the base pose by more than this."))
	float ChangedRotationTolerance = 0.01f;

	UPROPERTY(Config, EditAnywhere, Category = "Changed Bones", meta = (ClampMin = 0, Units = "cm", ToolTip = "A bone counts as changed by a pose when its translation differs from the base pose by more than this."))
	float ChangedTranslationTolerance = 0.01f;

	UPROPERTY(Config, EditAnywhere, Category = "Changed Bones", meta = (ClampMin = 0, ToolTip = "A bone counts as changed by a pose when its scale differs from the base pose by more than this."))
	float ChangedScaleTolerance = 0.001f;
};
