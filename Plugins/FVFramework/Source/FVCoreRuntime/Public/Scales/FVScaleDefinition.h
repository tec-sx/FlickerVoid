#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "GameplayTagContainer.h"
#include "FVScaleDefinition.generated.h"

class UFVScaleDefinition;

/** A named range of a scale, from its threshold up to the next band's. */
USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVScaleBand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale", meta = (ToolTip = "Value at or above which this band applies, up to the next band's threshold."))
	int32 Threshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale", meta = (ToolTip = "Identifies the band for conditions and game rules."))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale")
	FText Label;
};

/** Passes part of every change of one scale on to another. */
USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVScaleLink
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale")
	TObjectPtr<UFVScaleDefinition> Scale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale", meta = (ToolTip = "Fraction of the applied change passed on; negative to move the other scale the opposite way."))
	float Factor = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scale", meta = (ToolTip = "Pass on the size of the change whatever its direction, so a loss here still raises the other scale."))
	bool bMagnitude = false;
};

/**
 * A value kept in the fact named by Id, read through bands and changed through Modify Scale, which passes the change on
 * through Links. Range and default come from the fact's entry in the Facts settings.
 */
UCLASS(BlueprintType)
class FVCORERUNTIME_API UFVScaleDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** The band with the highest threshold the value reaches; null below every band. */
	const FFVScaleBand* FindBand(int32 Value) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scale", meta = (TitleProperty = "Label", MaxEntries = 3))
	TArray<FFVScaleBand> Bands;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scale", meta = (ToolTip = "Scales this one's applied changes feed, once each: a change passed on doesn't travel further.", MaxEntries = 3))
	TArray<FFVScaleLink> Links;

protected:
	virtual bool RequiresId() const override { return true; }
};
