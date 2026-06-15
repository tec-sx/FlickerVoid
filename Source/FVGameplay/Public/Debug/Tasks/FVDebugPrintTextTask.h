// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "FVDebugPrintTextTask.generated.h"

UENUM(BlueprintType, Category = "FV|Debug")
enum class EFVDebugParamType : uint8
{
	Float   UMETA(DisplayName = "Float"),
	Int     UMETA(DisplayName = "Int"),
	String  UMETA(DisplayName = "String"),
	Name    UMETA(DisplayName = "Name"),
	Text    UMETA(DisplayName = "Text"),
	Bool    UMETA(DisplayName = "Bool"),
};

USTRUCT(BlueprintType, Category = "PGAS|Debug")
struct FFVDebugFormatParam
{
	GENERATED_BODY()

	/** The placeholder key in your FormatText, e.g. "Health" for "{Health}" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FName Key;

	/** Which value slot to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	EFVDebugParamType Type = EFVDebugParamType::Float;

	/** Only valid if Type==Float */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	float FloatValue = 0.f;

	/** Only valid if Type==Int */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	int32 IntValue = 0;

	/** Only valid if Type==String */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FString StringValue;

	/** Only valid if Type==Name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FName NameValue;

	/** Only valid if Type==Text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FText TextValue;

	/** Only valid if Type==Bool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool BoolValue;
};

/**
 * Displays a formatted debug string using a printf-style format.
 * Example FormatString: "Health: %.2f"
 * Parameter will be substituted for the “%f”.
 */
UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVDebugPrintTextTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
	public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
    FText FormatText = NSLOCTEXT("FV", "DebugFmt", "Type Something {Value}");
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
    TArray<FFVDebugFormatParam> Params;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    TObjectPtr<AActor> Actor = nullptr;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task", meta = (DisplayName = "Print To Screen"))
    bool PrintToScreen = false;

#if WITH_EDITOR

    /**
     * Gets the description of the condition for editor display.
     * @param ID The unique identifier for the node.
     * @param InstanceDataView The data view for the instance.
     * @param BindingLookup The binding lookup for the state tree.
     * @param Formatting The formatting options for the description.
     * @return The formatted description text.
    */
    virtual FText GetDescription(
        const FGuid& ID,
        FStateTreeDataView InstanceDataView,
        const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const {
        return NSLOCTEXT(
            "FVSTInteraction",
            "InteractionEvaluatorDesc",
            "Display Formatted Debug Text at Actor Location"
        );
    };

#endif

protected:
    /**
      * This function will find a random reachable location around the actor within the specified radius.
      * @param Context The execution context for the state tree.
      * @param Transition The transition result containing information about the state change.
     */
    virtual EStateTreeRunStatus EnterState(
    	FStateTreeExecutionContext& Context, 
    	const FStateTreeTransitionResult& Transition) override;
};
