#pragma once
#include "CoreMinimal.h"
#include "Dialogue/FVDialogueContext.h"
#include "Dialogue/Conditions/FVDialogueConditionEvaluator.h"
#include "FVDialogueTaskInstanceData.generated.h"

USTRUCT(BlueprintType, 
	Category = "Flicker Void",
	meta = (DisplayName = "Dialogue Instance Data",
		Description = "Contains information about instance data."))
struct FFVDialogueTaskInstanceData
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere)
	FFVDialogueContext Context;

	UPROPERTY(EditAnywhere)
	FName CurrentLineID;

	const FFVDialogueLineRow* CurrentLineRow = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<FFVEvaluatedResponse> CurrentResponses;

	UPROPERTY(BlueprintReadOnly)
	FName ChosenResponseID;

	UPROPERTY(BlueprintReadOnly)
	TSet<FName> SessionConsumedLines;

	UPROPERTY(BlueprintReadOnly)
	bool bSessionEnded = false;
};