#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Scales/FVScaleDefinition.h"
#include "FVScaleLibrary.generated.h"

/** Reads and changes scales. Values live in facts, so they save, show in the facts debugger and fire fact change events. */
UCLASS()
class FVCORERUNTIME_API UFVScaleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Scale", meta = (WorldContext = "WorldContext"))
	static int32 GetValue(const UObject* WorldContext, const UFVScaleDefinition* Scale);

	UFUNCTION(BlueprintPure, Category = "FV|Scale", meta = (WorldContext = "WorldContext", ToolTip = "The band the current value falls in; false below every band."))
	static bool GetBand(const UObject* WorldContext, const UFVScaleDefinition* Scale, FFVScaleBand& OutBand);

	UFUNCTION(BlueprintPure, Category = "FV|Scale", meta = (ToolTip = "Min and Max of the scale's fact from the Facts settings; false when the fact has no range."))
	static bool GetRange(const UFVScaleDefinition* Scale, int32& OutMin, int32& OutMax);

	UFUNCTION(BlueprintCallable, Category = "FV|Scale", meta = (WorldContext = "WorldContext", ToolTip = "Adds Delta within the scale's range and passes the change that applied on through its links, once. Returns the change that applied."))
	static int32 ModifyScale(UObject* WorldContext, const UFVScaleDefinition* Scale, int32 Delta);

private:
	/** Returns the change that applied after the fact's range clamped it. */
	static int32 AddClamped(UObject* WorldContext, const UFVScaleDefinition* Scale, int32 Delta);
};
