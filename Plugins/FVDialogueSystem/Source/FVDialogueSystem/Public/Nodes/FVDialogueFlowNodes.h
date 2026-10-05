#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Nodes/FlowNode.h"
#include "FVDialogueFlowNodes.generated.h"

class UFVCharacterDefinition;
class UFVDialogueSubsystem;
class USoundBase;

/** Base for nodes that need the active conversation; opens one implicitly when the graph wasn't started by the subsystem. */
UCLASS(Abstract, NotBlueprintable)
class FVDIALOGUESYSTEM_API UFVFlowNode_DialogueBase : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_DialogueBase();

	virtual void Cleanup() override;

protected:
	UFVDialogueSubsystem* EnsureConversation();

	bool bPending = false;
};

/** A spoken line. Finishes after its duration, voice, or player input. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Line"))
class FVDIALOGUESYSTEM_API UFVFlowNode_DialogueLine : public UFVFlowNode_DialogueBase
{
	GENERATED_BODY()

public:
	UFVFlowNode_DialogueLine();

	/** Empty = the conversation owner (usually the NPC). */
	UPROPERTY(EditAnywhere, Category = "Line")
	TObjectPtr<UFVCharacterDefinition> Speaker;

	UPROPERTY(EditAnywhere, Category = "Line", meta = (MultiLine = true))
	FText Text;

	UPROPERTY(EditAnywhere, Category = "Line")
	TSoftObjectPtr<USoundBase> Voice;

	UPROPERTY(EditAnywhere, Category = "Line", meta = (Categories = "Dialogue.Mood"))
	FGameplayTag Mood;

	/** 0 = from voice length or text length. */
	UPROPERTY(EditAnywhere, Category = "Line", meta = (ClampMin = 0, Units = "s"))
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, Category = "Line")
	bool bSkippable = true;

	UPROPERTY(EditAnywhere, Category = "Line")
	bool bWaitForInput = false;

	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif

private:
	void HandleFinished();
};

USTRUCT(BlueprintType)
struct FVDIALOGUESYSTEM_API FFVDialogueChoiceOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Choice")
	FText Text;

	/** Failure presentation decides whether an unavailable option is hidden or shown locked. */
	UPROPERTY(EditAnywhere, Category = "Choice")
	FFVConditionSet Conditions;

	UPROPERTY(EditAnywhere, Category = "Choice")
	FFVEffectList OnChosen;
};

/** Player choice. One output pin per option. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Choice"))
class FVDIALOGUESYSTEM_API UFVFlowNode_DialogueChoice : public UFVFlowNode_DialogueBase
{
	GENERATED_BODY()

public:
	UFVFlowNode_DialogueChoice();

	UPROPERTY(EditAnywhere, Category = "Choice", meta = (TitleProperty = "Text"))
	TArray<FFVDialogueChoiceOption> Options;

	virtual void ExecuteInput(const FName& PinName) override;

	static FName GetOptionPinName(int32 Index);

#if WITH_EDITOR
	virtual bool SupportsContextPins() const override { return true; }
	virtual TArray<FFlowPin> GetContextOutputs() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FString GetNodeDescription() const override;
#endif

private:
	void HandleChosen(int32 Index);
};

/** Ends the conversation and finishes the graph. Use it instead of Finish in dialogue graphs. */
UCLASS(NotBlueprintable, meta = (DisplayName = "End Conversation"))
class FVDIALOGUESYSTEM_API UFVFlowNode_EndConversation : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_EndConversation();

	virtual void ExecuteInput(const FName& PinName) override;
	virtual bool CanFinishGraph() const override { return true; }
};

/** One bark line, filtered by speaker tags and conditions. */
USTRUCT(BlueprintType)
struct FVDIALOGUESYSTEM_API FFVBarkTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark")
	TSoftObjectPtr<USoundBase> Voice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bark", meta = (ClampMin = 0, Units = "s"))
	float DisplayDuration = 3.f;

	/** Speaker must own this tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	FGameplayTag IdentityTag;

	/** Checked against speaker and player tags combined. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	FGameplayTagContainer BlockingTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter")
	FFVConditionSet Conditions;
};

/** Picks a valid, non-repeating bark for the flow owner, broadcasts it and optionally waits for it. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Bark"))
class FVDIALOGUESYSTEM_API UFVFlowNode_Bark : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_Bark();

	UPROPERTY(EditAnywhere, Category = "Bark", meta = (RequiredAssetDataTags = "RowStructure=/Script/FVDialogueSystem.FVBarkTableRow"))
	TObjectPtr<UDataTable> Barks;

	UPROPERTY(EditAnywhere, Category = "Bark")
	bool bWaitForDuration = true;

	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

private:
	const FFVBarkTableRow* PickRow(AActor* Speaker, FName& OutRowName) const;
	void HandleFinished();

	FTimerHandle Timer;
	TWeakObjectPtr<class UAudioComponent> PlayingVoice;

	static TMap<TWeakObjectPtr<AActor>, FName> LastRowBySpeaker;
};
